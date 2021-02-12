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


#ifndef _INCLUDE_METASYS_NET_SOCKET_HXX_
#define _INCLUDE_METASYS_NET_SOCKET_HXX_


#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cerrno>

#include <utility>

#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


template<int Domain, int Type, int Protocol>
class Socket
{
	FileDescriptor  _fd;


 protected:
	constexpr Socket(int fd) noexcept
		: _fd(fd)
	{
	}


	int fd() const noexcept
	{
		return _fd.value();
	}


	template<typename ErrHandler>
	auto setsockopt(int level, int optname, const void *optval,
			socklen_t optlen, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return handler(::setsockopt(fd(), level, optname,
					    optval, optlen));
	}

	template<typename Value, typename ErrHandler>
	auto setsockopt(int level, int optname, const Value &optval,
			ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return setsockopt(level, optname, &optval, sizeof (optval),
				  std::forward<ErrHandler>(handler));
	}


 public:
	constexpr Socket() noexcept = default;
	Socket(const Socket &other) = delete;
	Socket(Socket &&other) noexcept = default;

	Socket &operator=(const Socket &other) = delete;
	Socket &operator=(Socket &&other) noexcept = default;


	constexpr bool valid() const noexcept
	{
		return _fd.valid();
	}


	template<typename ErrHandler>
	auto open(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);

		_fd.reset(::socket(Domain, Type, Protocol));

		return handler(fd());
	}

	void open()
	{
		open([](int ret) {
			if (ret < 0) [[unlikely]]
				openthrow();
		});
	}

	static void openthrow()
	{
		assert(errno != EINVAL);

		SystemException::throwErrno();
	}


	template<typename ... Args>
	auto close(Args && ... args)
		noexcept (noexcept (_fd.close(std::forward<Args>(args) ...)))
	{
		return _fd.close(std::forward<Args>(args) ...);
	}
};


}


#endif
