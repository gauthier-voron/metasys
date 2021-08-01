// Copyright 2021 Gauthier Voron
//
// This file is part of Metasys.
//
// Metasys is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Metasys is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// Metasys. If not, see <https://www.gnu.org/licenses/>.
//


#ifndef _INCLUDE_METASYS_SCHED_PTHREAD_HXX_
#define _INCLUDE_METASYS_SCHED_PTHREAD_HXX_


#include <cassert>
#include <cerrno>

#include <pthread.h>

#include <concepts>
#include <exception>
#include <functional>
#include <type_traits>

#include <metasys/sched/PthreadBehavior.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


namespace detail {


template<typename T>
concept PointerTransferable = (sizeof (T) <= sizeof (void *));

struct PthreadRoutineArg
{
	template<typename T>
	requires PointerTransferable<T>
	constexpr operator T() const noexcept;
};

template<typename T>
concept PthreadRoutine = (sizeof (T) == sizeof (void *(*)(void *))) &&
	(std::invocable<T, PthreadRoutineArg> || std::invocable<T>);

template<typename T, typename Ret, typename ... Args>
concept InvokeCompatible =
	(std::invocable<T, Args ...> &&
	 (std::same_as<std::invoke_result_t<T, Args ...>, Ret> ||
	  std::same_as<Ret, void>));


template<typename Routine, typename Ret>
concept PthreadNoArgCall = PthreadRoutine<Routine>
	&& InvokeCompatible<Routine, Ret>;

template<typename Routine, typename Ret, typename Arg>
concept PthreadArgCall = PthreadRoutine<Routine>
	&& PointerTransferable<Arg>
	&& InvokeCompatible<Routine, Ret, Arg>;


}


template<
	typename JoinRet = void *,
	PthreadBehavior Behavior = PthreadBehavior::Terminate
	>
requires (detail::PointerTransferable<JoinRet> || std::same_as<JoinRet, void>)
class Pthread
{
	static const pthread_t INVALID = 0;


	template<typename T>
	requires detail::PointerTransferable<T>
	union PtrCast
	{
		T      value;
		void  *ptr;
	};

	template<typename T>
	static constexpr void *ptr_cast(T value) noexcept
	{
		return ((PtrCast<T>) { .value = value }).ptr;
	}

	template<typename T>
	static constexpr T value_cast(void *ptr) noexcept
	{
		return ((PtrCast<T>) { .ptr = ptr }).value;
	}


	using RoutineType = void *(*)(void *);

	template<typename T>
	requires detail::PthreadRoutine<T>
	union RoutineCast
	{
		T            origin;
		RoutineType  routine;
	};

	template<typename T>
	static constexpr RoutineType routine_cast(T origin) noexcept
	{
		return ((RoutineCast<T>) { .origin = origin }).routine;
	}


	pthread_t  _tid;


	void _stop() noexcept
	{
		if constexpr (Behavior & PthreadBehavior::Cancel) {
			trycancel();
		} else if constexpr (Behavior.signum() != 0) {
			kill(Behavior.signum());
		}

		if constexpr (Behavior & PthreadBehavior::Terminate) {
			if (valid())
				std::terminate();
		} else if constexpr (Behavior & PthreadBehavior::Detach) {
			::pthread_detach(_tid);
		} else if constexpr (Behavior & PthreadBehavior::Join) {
			::pthread_join(_tid, NULL);
		} else {
			static_assert (Behavior & PthreadBehavior::Nothing);
		}
	}


	template<auto Method, typename Obj>
	static JoinRet _wrapper(void *arg)
	{
		if constexpr (std::same_as<JoinRet, void>) {
			std::invoke(Method, value_cast<Obj>(arg));
		} else {
			return std::invoke(Method, value_cast<Obj>(arg));
		}
	}

	template<auto Method>
	static JoinRet _wrapper(void *)
	{
		return std::invoke(Method);
	}


 public:
	constexpr Pthread() noexcept
		: _tid(INVALID)
	{
	}

	constexpr Pthread(pthread_t tid) noexcept
		: _tid(tid)
	{
	}

	Pthread(const Pthread &other) = delete;

	Pthread(Pthread &&other) noexcept
		: _tid(other._tid)
	{
		other._tid = INVALID;
	}

	~Pthread()
	{
		_stop();
	}

	Pthread &operator=(const Pthread &other) = delete;

	Pthread &operator=(Pthread &&other) noexcept
	{
		if (pthread_equal(_tid, other._tid) != 0) [[likely]]
			_stop();

		_tid = other._tid;
		other._tid = INVALID;

		return *this;
	}

	pthread_t reset(pthread_t tid = INVALID) noexcept
	{
		pthread_t tmp = _tid;

		_tid = tid;

		return tmp;
	}

	bool valid() const noexcept
	{
		return (pthread_equal(_tid, INVALID) == 0);
	}


	// ====================================================================

	template<typename Routine, typename Arg, typename ErrHandler>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	      && std::invocable<ErrHandler, int>
	auto create(const pthread_attr_t *attr, Routine routine, Arg arg,
		    ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int ret;

		assert(valid() == false);

		ret = ::pthread_create(&_tid, attr,
				       routine_cast<Routine>(routine),
				       ptr_cast<Arg>(arg));

		return handler(ret);
	}

	template<typename Routine, typename Arg, typename ErrHandler>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(const pthread_attr_t *attr, Routine routine,
				  Arg arg, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		pthread_t tid;

		handler(::pthread_create(&tid, attr,
					 routine_cast<Routine>(routine),
					 ptr_cast<Arg>(arg)));

		return Pthread(tid);
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<typename Routine, typename ErrHandler>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	      && std::invocable<ErrHandler, int>
	auto create(const pthread_attr_t *attr, Routine routine,
		    ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int ret;

		assert(valid() == false);

		ret = ::pthread_create(&_tid, attr,
				       routine_cast<Routine>(routine), NULL);

		return handler(ret);
	}

	template<typename Routine, typename ErrHandler>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(const pthread_attr_t *attr, Routine routine,
				  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		pthread_t tid;

		handler(::pthread_create(&tid, attr,
					 routine_cast<Routine>(routine),
					 NULL));

		return Pthread(tid);
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method, typename Obj, typename ErrHandler>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	      && std::invocable<ErrHandler, int>
	auto create(const pthread_attr_t *attr, Obj obj, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int ret;

		assert(valid() == false);

		ret = ::pthread_create(&_tid, attr,
				       routine_cast(_wrapper<Method, Obj>),
				       ptr_cast<Obj>(obj));

		return handler(ret);
	}

	template<auto Method, typename Obj, typename ErrHandler>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(const pthread_attr_t *attr, Obj obj,
				  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		pthread_t tid;

		handler(::pthread_create(&tid, attr,
					 routine_cast(_wrapper<Method, Obj>),
					 ptr_cast<Obj>(obj)));

		return Pthread(tid);
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method, typename ErrHandler>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	      && std::invocable<ErrHandler, int>
	auto create(const pthread_attr_t *attr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int ret;

		assert(valid() == false);

		ret = ::pthread_create(&_tid, attr,
				       routine_cast(_wrapper<Method>), NULL);

		return handler(ret);
	}

	template<auto Method, typename ErrHandler>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(const pthread_attr_t *attr,
				  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		pthread_t tid;

		handler(::pthread_create(&tid, attr,
					 routine_cast(_wrapper<Method>),
					 NULL));

		return Pthread(tid);
	}

	// --------------------------------------------------------------------

	template<typename Routine, typename Arg>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	void create(const pthread_attr_t *attr, Routine routine, Arg arg)
	{
		create(attr, routine, arg, [&](int ret) {
			if (ret != 0) [[unlikely]] {
				reset();
				createthrow(ret);
			}
		});
	}

	template<typename Routine, typename Arg>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	static Pthread createinit(const pthread_attr_t *attr, Routine routine,
				  Arg arg)
	{
		return createinit(attr, routine, arg, [](int ret) {
			if (ret != 0) [[unlikely]]
				createthrow(ret);
		});
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<typename Routine>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	      && std::same_as<std::invoke_result_t<Routine>, JoinRet>
	void create(const pthread_attr_t *attr, Routine routine)
	{
		create(attr, routine, [&](int ret) {
			if (ret != 0) [[unlikely]] {
				reset();
				createthrow(ret);
			}
		});
	}

	template<typename Routine>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	static Pthread createinit(const pthread_attr_t *attr, Routine routine)
	{
		return createinit(attr, routine, [](int ret) {
			if (ret != 0) [[unlikely]]
				createthrow(ret);
		});
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method, typename Obj>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	void create(const pthread_attr_t *attr, Obj obj)
	{
		create<Method>(attr, obj, [&](int ret) {
			if (ret != 0) [[unlikely]] {
				reset();
				createthrow(ret);
			}
		});
	}

	template<auto Method, typename Obj>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	static Pthread createinit(const pthread_attr_t *attr, Obj obj)
	{
		return createinit<Method>(attr, obj, [](int ret) {
			if (ret != 0) [[unlikely]]
				createthrow(ret);
		});
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	void create(const pthread_attr_t *attr)
	{
		create<Method>(attr, [&](int ret) {
			if (ret != 0) [[unlikely]] {
				reset();
				createthrow(ret);
			}
		});
	}

	template<auto Method>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	static Pthread createinit(const pthread_attr_t *attr)
	{
		return createinit<Method>(attr, [](int ret) {
			if (ret != 0) [[unlikely]]
				createthrow(ret);
		});
	}

	// ====================================================================

	template<typename Routine, typename Arg, typename ErrHandler>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	      && std::invocable<ErrHandler, int>
	auto create(Routine routine, Arg arg, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return create(NULL, routine, arg,
			      std::forward<ErrHandler>(handler));
	}

	template<typename Routine, typename Arg, typename ErrHandler>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(Routine routine, Arg arg,
				  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return createinit(NULL, routine, arg,
				  std::forward<ErrHandler>(handler));
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<typename Routine, typename ErrHandler>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	      && std::invocable<ErrHandler, int>
	auto create(Routine routine, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return create(NULL, routine,
			      std::forward<ErrHandler>(handler));
	}

	template<typename Routine, typename ErrHandler>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(Routine routine, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return createinit(NULL, routine,
				  std::forward<ErrHandler>(handler));
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method, typename Obj, typename ErrHandler>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	      && std::invocable<ErrHandler, int>
	auto create(Obj obj, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return create<Method>(NULL, obj,
				      std::forward<ErrHandler>(handler));
	}

	template<auto Method, typename Obj, typename ErrHandler>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(Obj obj, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return createinit<Method>(NULL, obj,
					  std::forward<ErrHandler>(handler));
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method, typename ErrHandler>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	      && std::invocable<ErrHandler, int>
	auto create(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return create<Method>(NULL, std::forward<ErrHandler>(handler));
	}

	template<auto Method, typename ErrHandler>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	      && std::invocable<ErrHandler, int>
	static Pthread createinit(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return createinit<Method>(NULL,
					  std::forward<ErrHandler>(handler));
	}

	// --------------------------------------------------------------------

	template<typename Routine, typename Arg>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	void create(Routine routine, Arg arg)
	{
		create(NULL, routine, arg);
	}

	template<typename Routine, typename Arg>
	requires detail::PthreadArgCall<Routine, JoinRet, Arg>
	static Pthread createinit(Routine routine, Arg arg)
	{
		return createinit(NULL, routine, arg);
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<typename Routine>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	      && std::same_as<std::invoke_result_t<Routine>, JoinRet>
	void create(Routine routine)
	{
		create(NULL, routine);
	}

	template<typename Routine>
	requires detail::PthreadNoArgCall<Routine, JoinRet>
	static Pthread createinit(Routine routine)
	{
		return createinit(NULL, routine);
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method, typename Obj>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	void create(Obj obj)
	{
		create<Method>(NULL, obj);
	}

	template<auto Method, typename Obj>
	requires detail::PointerTransferable<Obj>
	      && detail::InvokeCompatible<decltype (Method), JoinRet, Obj>
	static Pthread createinit(Obj obj)
	{
		return createinit<Method>(NULL, obj);
	}

	//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<auto Method>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	void create()
	{
		create<Method>(NULL);
	}

	template<auto Method>
	requires detail::InvokeCompatible<decltype (Method), JoinRet>
	static Pthread createinit()
	{
		return createinit<Method>(NULL);
	}

	// ====================================================================

	static void createthrow(int ret)
	{
		assert(ret != EINVAL);

		SystemException::throwErrno(ret);
	}


	template<typename ErrHandler>
	JoinRet join(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		if constexpr (std::same_as<JoinRet, void>) {
			handler(::pthread_join(_tid, NULL));

			_tid = INVALID;
		} else {
			PtrCast<JoinRet> jret;

			handler(::pthread_join(_tid, (void **) &jret.ptr));

			_tid = INVALID;

			return jret.value;
		}
	}

	JoinRet join() noexcept
	{
		return join([](int ret) {
			if (ret != 0) [[unlikely]]
				jointhrow(ret);
		});
	}

	static void jointhrow(int ret [[maybe_unused]]) noexcept
	{
		assert(ret != EDEADLK);
		assert(ret != EINVAL);
		assert(ret != ESRCH);
	}


	template<typename ErrHandler>
	auto detach(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int ret;

		assert(valid());

		ret = ::pthread_detach(_tid);

		_tid = INVALID;

		return handler(ret);
	}

	void detach() noexcept
	{
		detach([](int ret) {
			if (ret != 0) [[unlikely]]
				detachthrow(ret);
		});
	}

	static void detachthrow(int ret [[maybe_unused]]) noexcept
	{
		assert(ret != EINVAL);
		assert(ret != ESRCH);
	}


	template<typename ErrHandler>
	auto cancel(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::pthread_cancel(_tid));
	}

	void cancel() noexcept
	{
		cancel([](int ret) {
			if (ret != 0) [[unlikely]]
				cancelthrow(ret);
		});
	}

	bool trycancel() noexcept
	{
		return cancel([](int ret) {
			return (ret == 0);
		});
	}

	static void cancelthrow(int ret [[maybe_unused]]) noexcept
	{
		assert(ret != ESRCH);
	}
};


}


#endif
