#ifndef _INCLUDE_METASYS_SCHED_PROCESS_HXX_
#define _INCLUDE_METASYS_SCHED_PROCESS_HXX_


#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <functional>

#include <metasys/meta/AutoHolder.hxx>
#include <metasys/sched/ProcessBehavior.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


template<ProcessBehavior Behavior = ProcessBehavior::DEFAULT()>
class Process
{
	pid_t   _pid;


	void _stop() noexcept
	{
		assert(valid());

		if constexpr (Behavior.SignalOnDestroy != 0) {
			kill(Behavior.SignalOnDestroy, [](int){});
		}

		if constexpr (Behavior.WaitOnDestroy) {
			wait(0, [](pid_t){});
		}
	}


 public:
	constexpr Process() noexcept
		: _pid(0)
	{
	}

	constexpr Process(pid_t pid) noexcept
		: _pid(pid)
	{
	}

	Process(const Process &other) = delete;

	Process(Process &&other) noexcept
		: _pid(other._pid)
	{
		other._pid = 0;
	}

	~Process()
	{
		if (valid())
			_stop();
	}

	Process &operator=(const Process &other) = delete;

	Process &operator=(Process &&other) noexcept
	{
		pid_t tmp;

		if (valid())
			if (_pid != other._pid) [[likely]]
				_stop();

		tmp = other._pid;
		other._pid = 0;
		_pid = tmp;

		return *this;
	}


	constexpr pid_t pid() const noexcept
	{
		return _pid;
	}

	constexpr bool valid() const noexcept
	{
		return (_pid > 0);
	}

	int reset(pid_t pid = 0) noexcept
	{
		pid_t ret = _pid;

		_pid = pid;

		return ret;
	}


	template<typename ErrHandler>
	auto fork(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);

		_pid = ::fork();

		return handler(_pid);
	}

	void fork()
	{
		fork([](pid_t ret) {
			if (ret < 0) [[unlikely]]
				throwfork();
		});
	}

	template<typename ErrHandler>
	static Process forkinit(ErrHandler &&handler)
	{
		pid_t pid = ::fork();

		handler(pid);

		return Process(pid);
	}

	static Process forkinit()
	{
		return forkinit([](pid_t ret) {
			if (ret < 0) [[unlikely]]
				throwfork();
		});
	}


	// TODO: should work but not tested yet

	// template<typename ErrHandler, typename Function, typename ... Args>
	// requires std::invocable<Function, Args ...>
	//       && std::invocable<ErrHandler, pid_t>
	// auto forkto(Function &&function, Args && ... args,
	// 	    ErrHandler &&handler)
	// 	noexcept (noexcept (handler(-1)))
	// {
	// 	details::AutoHolder ret =
	// 		details::AutoHolder([](Process *t, ErrHandler &&h) {
	// 		return t->fork(std::forward<ErrHandler>(h));
	// 	}, this, std::forward<ErrHandler>(handler));

	// 	if (_pid == 0) {
	// 		std::invoke(std::forward<Function>(function),
	// 			    std::forward<Args>(args) ...);
	// 		::exit(0);
	// 	}

	// 	if constexpr (ret.isvoid() == false) {
	// 		return ret.value();
	// 	}
	// }

	// template<typename Function, typename ... Args>
	// requires std::invocable<Function, Args ...>
	// auto forkto(Function &&function, Args && ... args)
	// {
	// 	return forkto(std::forward<Function>(function),
	// 		      std::forward<Args>(args) ...,
	// 		      [](pid_t ret) {
	// 			      if (ret < 0) [[unlikely]]
	// 				      throwfork();
	// 		      });
	// }

	// template<typename ErrHandler, typename Function, typename ... Args>
	// requires std::invocable<Function, Args ...>
	//       && std::invocable<ErrHandler, pid_t>
	// static Process forktoinit(Function &&function, Args && ... args,
	// 			  ErrHandler &&handler)
	// 	noexcept (noexcept (handler(-1)))
	// {
	// 	Process ret = forkinit(std::forward<ErrHandler>(handler));

	// 	if (ret.pid() == 0) {
	// 		std::invoke(std::forward<Function>(function),
	// 			    std::forward<Args>(args) ...);
	// 		::exit(0);
	// 	}

	// 	return ret;
	// }

	// template<typename Function, typename ... Args>
	// requires std::invocable<Function, Args ...>
	// static Process forktoinit(Function &&function, Args && ... args)
	// {
	// 	return forktoinit(std::forward<Function>(function),
	// 			  std::forward<Args>(args) ...,
	// 			  [](pid_t ret) {
	// 				  if (ret < 0) [[unlikely]]
	// 					  throwfork();
	// 			  });
	// }


	static void throwfork()
	{
		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto wait(int *wstatus, int options, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::waitpid(_pid, wstatus, options));
	}

	template<typename ErrHandler>
	auto wait(int *wstatus, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return wait(wstatus, 0, std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto wait(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return wait(nullptr, std::forward<ErrHandler>(handler));
	}

	void wait(int *wstatus = nullptr, int options = 0)
	{
		wait(wstatus, options, [](pid_t ret) {
			if (ret < 0) [[unlikely]]
				throwwait();
		});
	}

	static void throwwait()
	{
		assert(errno != ECHILD);
		assert(errno != EINVAL);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto kill(int sig, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::kill(_pid, sig));
	}

	void kill(int sig)
	{
		kill(sig, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwkill();
		});
	}

	static void throwkill()
	{
		assert(errno != EINVAL);

		SystemException::throwErrno();
	}
};


}


#endif
