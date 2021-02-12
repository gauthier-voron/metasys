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


#ifndef _INCLUDE_METASYS_NET_TCPSOCKET_HXX_
#define _INCLUDE_METASYS_NET_TCPSOCKET_HXX_


#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>

#include <concepts>
#include <utility>

#include <metasys/net/InetAddress.hxx>
#include <metasys/net/TcpBaseSocket.hxx>


namespace metasys {


class TcpSocket : public TcpBaseSocket
{
	friend class TcpServerSocket;


 protected:
	constexpr TcpSocket(int fd) noexcept
		: TcpBaseSocket(fd)
	{
	}


 public:
	constexpr TcpSocket() noexcept = default;
	TcpSocket(const TcpSocket &other) = delete;
	TcpSocket(TcpSocket &&other) noexcept = default;

	TcpSocket &operator=(const TcpSocket &other) = delete;
	TcpSocket &operator=(TcpSocket &&other) noexcept = default;


	template<typename ErrHandler>
	auto connect(const struct sockaddr_in *addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		const struct sockaddr *saddr;
		int ret;

		assert(valid());

		saddr = reinterpret_cast<const struct sockaddr *> (addr);
		ret = ::connect(fd(), saddr, sizeof (*addr));

		return handler(ret);
	}

	template<typename ErrHandler>
	auto connect(const InetAddress &addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return connect(addr, std::forward<ErrHandler>(handler));
	}

	void connect(const InetAddress &addr);
	void connect(const struct sockaddr_in *addr);

	static void connectthrow();


	template<typename ErrHandler>
	auto disconnect(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		struct sockaddr saddr;
		int ret;

		assert(valid());

		saddr.sa_family = AF_UNSPEC;
		ret = ::connect(fd(), &saddr, sizeof (saddr));

		return handler(ret);
	}

	void disconnect();

	static void disconnectthrow();


	template<typename ErrHandler>
	auto read(void *dest, size_t len, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return handler(::read(fd(), dest, len));
	}

	size_t read(void *dest, size_t len);

	static void readthrow();


	template<typename ErrHandler>
	auto write(const void *src, size_t len, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return handler(::write(fd(), src, len));
	}

	size_t write(const void *src, size_t len);

	static void writethrow();


	template<typename ErrHandler>
	static TcpSocket instance(const struct sockaddr_in *addr,
				  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		TcpSocket sock;
		int ret;

		if ((ret = sock.open([](int ret) { return ret; })) < 0)
			goto err;

		if ((ret = sock.connect(addr, [](int r) { return r; })) < 0)
			goto err;

		return sock;
	 err:
		handler(ret);
		return TcpSocket();
	}

	template<typename ErrHandler>
	static TcpSocket instance(const InetAddress &addr,
				  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return instance(addr, std::forward<ErrHandler>(handler));
	}

	static TcpSocket instance(const InetAddress &addr);
	static TcpSocket instance(const struct sockaddr_in *addr);
};


}


#endif
