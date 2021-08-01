#ifndef _INCLUDE_METASYS_SCHED_SIGACTION_HXX_
#define _INCLUDE_METASYS_SCHED_SIGACTION_HXX_


#include <cassert>
#include <cerrno>
#include <csignal>

#include <concepts>
#include <utility>


namespace metasys {


class Sigaction : public sigaction
{
	bool _addmask(int signum) noexcept
	{
		addmask(signum);
		return true;
	}


 public:
	Sigaction() noexcept = default;

	template<int ... Signals>
	struct Mask
	{
		constexpr Mask() noexcept = default;
	};

	Sigaction(void (*handler)(int), int flags = 0)
	{
		sa_handler = handler;
		sa_flags = flags;
		emptymask();
	}

	template<int ... Signals>
	Sigaction(void (*handler)(int), int flags, Mask<Signals...>)
	{
		sa_handler = handler;
		sa_flags = flags;

		emptymask();

		if ((_addmask(Signals) && ... && true))
			{}
	}

	Sigaction(void (*handler)(int, siginfo_t *, void *), int flags = 0)
	{
		sa_sigaction = handler;
		sa_flags = flags | SA_SIGINFO;
		emptymask();
	}

	template<int ... Signals>
	Sigaction(void (*handler)(int, siginfo_t *, void *), int flags,
		  Mask<Signals ...>)
	{
		sa_sigaction = handler;
		sa_flags = flags | SA_SIGINFO;

		emptymask();

		if ((_addmask(Signals) && ... && true))
			{}
	}

	Sigaction(const Sigaction &other) noexcept = default;
	Sigaction(Sigaction &&other) noexcept = default;

	Sigaction &operator=(const Sigaction &other) noexcept = default;
	Sigaction &operator=(Sigaction &&other) noexcept = default;


	void operator()(int sig, siginfo_t *info, void *ucontext)
	{
		if ((sa_flags & SA_SIGINFO) != 0) {
			sa_sigaction(sig, info, ucontext);
		} else {
			assert(sa_handler != SIG_DFL);
			assert(sa_handler != SIG_IGN);

			sa_handler(sig);
		}
	}

	bool isdfl() const noexcept
	{
		if ((sa_flags & SA_SIGINFO) != 0)
			return false;
		else
			return (sa_handler == SIG_DFL);
	}

	bool isign() const noexcept
	{
		if ((sa_flags & SA_SIGINFO) != 0)
			return false;
		else
			return (sa_handler == SIG_IGN);
	}


	void emptymask() noexcept
	{
		::sigemptyset(&sa_mask);    // only documented error impossible
	}

	void fillmask() noexcept
	{
		::sigfillset(&sa_mask);     // only documented error impossible
	}

	void addmask(int signum) noexcept
	{
		int ret [[maybe_unused]];

		ret = ::sigaddset(&sa_mask, signum);

		assert(ret == 0);
	}

	void delmask(int signum) noexcept
	{
		int ret [[maybe_unused]];

		ret = ::sigdelset(&sa_mask, signum);

		assert(ret == 0);
	}

	bool ismasked(int signum) const noexcept
	{
		int ret;

		ret = ::sigismember(&sa_mask, signum);

		assert(ret >= 0);

		return (ret != 0);
	}


	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto install(int signum, struct sigaction *old, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return handler(::sigaction
			       (signum, static_cast<struct sigaction *> (this),
				old));
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto install(int signum, Sigaction *old, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return install(signum, static_cast<struct sigaction *> (old),
			       std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto install(int signum, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return install(signum, (struct sigaction *) nullptr,
			       std::forward<ErrHandler>(handler));
	}

	template<typename Old>
	void install(int signum, Old *old) noexcept
	{
		install(signum, old, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwinstall();
		});
	}

	void install(int signum) noexcept
	{
		install(signum, (struct sigaction *) nullptr);
	}

	static void throwinstall() noexcept
	{
		assert(errno != EFAULT);
		assert(errno != EINVAL);
	}


	template<typename ErrHandler>
	auto save(int signum, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return handler(::sigaction
			       (signum, nullptr,
				static_cast<struct sigaction *> (this)));
	}

	void save(int signum) noexcept
	{
		save(signum, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwsave();
		});
	}

	template<typename ErrHandler>
	static Sigaction saveinit(int signum, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		Sigaction ret;

		ret.save(signum, std::forward<ErrHandler>(handler));

		return ret;
	}

	static Sigaction saveinit(int signum) noexcept
	{
		return saveinit(signum, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwsave();
		});
	}

	static void throwsave() noexcept
	{
		assert(errno != EFAULT);
		assert(errno != EINVAL);
	}
};


}


#endif
