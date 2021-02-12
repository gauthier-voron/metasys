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


#ifndef _INCLUDE_METASYS_SCHED_PTHREADMUTEX_HXX_
#define _INCLUDE_METASYS_SCHED_PTHREADMUTEX_HXX_


#include <pthread.h>

#include <cassert>
#include <cerrno>

#include <metasys/sys/SystemException.hxx>


namespace metasys {


class PthreadMutex
{
	pthread_mutex_t  _inner;


 public:
	constexpr PthreadMutex() noexcept
		: _inner(PTHREAD_MUTEX_INITIALIZER)
	{
	}

	PthreadMutex(const PthreadMutex &) = delete;
	PthreadMutex(PthreadMutex &&) = delete;

	~PthreadMutex()
	{
		int ret = ::pthread_mutex_destroy(&_inner);

		assert(ret == 0);
	}

	PthreadMutex &operator=(const PthreadMutex &) = delete;
	PthreadMutex &operator=(PthreadMutex &&) = delete;

	static void initthrow(int err)
	{
		assert(err != EINVAL);

		SystemException::throwErrno(err);
	}


	template<typename ErrHandler>
	auto lock(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		return handler(::pthread_mutex_lock(&_inner));
	}

	void lock() noexcept
	{
		lock([](int ret) {
			if (ret != 0) [[unlikely]]
				lockthrow(ret);
		});
	}

	static void lockthrow(int err [[maybe_unused]]) noexcept
	{
		assert(err != EAGAIN);
		assert(err != EBUSY);
		assert(err != EDEADLK);
		assert(err != EINVAL);
		assert(err != ENOTRECOVERABLE);
		assert(err != EOWNERDEAD);
		assert(err != EPERM);
	}


	template<typename ErrHandler>
	auto trylock(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		return handler(::pthread_mutex_trylock(&_inner));
	}

	bool trylock() noexcept
	{
		return trylock([](int ret) {
			if ((ret != 0) && (ret != EBUSY))
				trylockthrow(ret);
			return (ret == 0);
		});
	}

	static void trylockthrow(int err) noexcept
	{
		assert(err != EAGAIN);
		assert(err != EDEADLK);
		assert(err != EINVAL);
		assert(err != ENOTRECOVERABLE);
		assert(err != EOWNERDEAD);
		assert(err != EPERM);
	}


	template<typename ErrHandler>
	auto unlock(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		return handler(::pthread_mutex_unlock(&_inner));
	}

	void unlock() noexcept
	{
		unlock([](int ret) {
			if (ret != 0) [[unlikely]]
				unlockthrow(ret);
		});
	}

	static void unlockthrow(int err [[maybe_unused]]) noexcept
	{
		assert(err != EAGAIN);
		assert(err != EBUSY);
		assert(err != EDEADLK);
		assert(err != EINVAL);
		assert(err != ENOTRECOVERABLE);
		assert(err != EOWNERDEAD);
		assert(err != EPERM);
	}
};


}


#endif
