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


#ifndef _INCLUDE_METASYS_NET_TCPBASESOCKET_HXX_
#define _INCLUDE_METASYS_NET_TCPBASESOCKET_HXX_


#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <metasys/net/InetAddress.hxx>
#include <metasys/net/Socket.hxx>


namespace metasys {


class TcpBaseSocket : public Socket<AF_INET, SOCK_STREAM, IPPROTO_TCP>
{
 protected:
	constexpr TcpBaseSocket(int fd) noexcept
		: Socket(fd)
	{
	}


 public:
	constexpr TcpBaseSocket() noexcept = default;
	TcpBaseSocket(const TcpBaseSocket &other) = delete;
	TcpBaseSocket(TcpBaseSocket &&other) noexcept = default;

	TcpBaseSocket &operator=(const TcpBaseSocket &other) = delete;
	TcpBaseSocket &operator=(TcpBaseSocket &&other) noexcept = default;


	template<typename ErrHandler>
	auto bind(const struct sockaddr_in *addr, ErrHandler &&handler)
	{
		const sockaddr *saddr;

		assert(valid());

		saddr = reinterpret_cast<const struct sockaddr *> (addr);
		return handler(::bind(fd(), saddr, sizeof (*addr)));
	}

	template<typename ErrHandler>
	auto bind(const InetAddress &addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return bind(addr.saddrin(), std::forward<ErrHandler>(handler));
	}

	void bind(const InetAddress &addr);
	void bind(const struct sockaddr_in *addr);

	static void bindthrow();


	template<typename ErrHandler>
	auto sockname(struct sockaddr_in *addr, ErrHandler &&handler) const
		noexcept (noexcept (handler(-1)))
	{
		struct sockaddr *saddr;
		socklen_t slen;

		assert(valid());

		saddr = reinterpret_cast<struct sockaddr *> (addr);
		slen = sizeof (*addr);
		return handler(::getsockname(fd(), saddr, &slen));
	}

	template<typename ErrHandler>
	auto sockname(InetAddress *addr, ErrHandler &&handler) const
		noexcept (noexcept (handler(-1)))
	{
		return sockname(addr->saddrin(),
				std::forward<ErrHandler>(handler));
	}

	InetAddress sockname() const;
	void sockname(struct sockaddr_in *dest) const;

	static void socknamethrow();


	template<typename ErrHandler>
	auto setreuseaddr(bool allow, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return setsockopt(SOL_SOCKET, SO_REUSEADDR,
				  static_cast<int> (allow),
				  std::forward<ErrHandler>(handler));
	}

	void setreuseaddr(bool allow) noexcept;

	static void setsockoptthrow() noexcept;
};



}


#endif
