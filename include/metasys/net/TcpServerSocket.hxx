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


#ifndef _INCLUDE_METASYS_NET_TCPSERVERSOCKET_HXX_
#define _INCLUDE_METASYS_NET_TCPSERVERSOCKET_HXX_


#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cerrno>

#include <metasys/net/InetAddress.hxx>
#include <metasys/net/TcpBaseSocket.hxx>
#include <metasys/net/TcpSocket.hxx>


namespace metasys {


class TcpServerSocket : public TcpBaseSocket
{
 public:
	constexpr TcpServerSocket() noexcept = default;
	TcpServerSocket(const TcpServerSocket &other) = delete;
	TcpServerSocket(TcpServerSocket &&other) noexcept = default;

	TcpServerSocket &operator=(const TcpServerSocket &other) = delete;
	TcpServerSocket &operator=(TcpServerSocket &&other) noexcept = default;


	template<typename ErrHandler>
	auto listen(int backlog, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::listen(fd(), backlog));
	}

	void listen(int backlog);

	static void listenthrow();


	template<typename ErrHandler>
	TcpSocket accept(struct sockaddr_in *from, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		socklen_t slen = sizeof (*from);
		int ret;

		assert(valid());

		ret = ::accept(fd(),
			       reinterpret_cast<struct sockaddr *> (from),
			       &slen);

		handler(ret);

		return TcpSocket(ret);
	}

	template<typename ErrHandler>
	TcpSocket accept(InetAddress *from, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return accept(from->saddrin(),
			      std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	TcpSocket accept(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return accept(reinterpret_cast<struct sockaddr_in *> (NULL),
			      std::forward<ErrHandler>(handler));
	}

	TcpSocket accept(InetAddress *from);
	TcpSocket accept(struct sockaddr_in *from);
	TcpSocket accept();

	static void acceptthrow();


	template<typename ErrHandler, bool ReuseAddr = true>
	requires std::invocable<ErrHandler, int>
	static TcpServerSocket instance(const struct sockaddr_in *addr,
					int backlog, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		TcpServerSocket sock;
		int ret;

		if ((ret = sock.open([](int ret) { return ret; })) < 0)
			goto err;

		if constexpr (ReuseAddr) {
			ret = sock.setreuseaddr(true, [](int r) { return r; });
			if (ret < 0)
				goto err;
		}

		if ((ret = sock.bind(addr, [](int ret) { return ret; })) < 0)
			goto err;

		if ((ret = sock.listen(backlog, [](int r) { return r; })) < 0)
			goto err;

		return sock;
	 err:
		handler(ret);
		return TcpServerSocket();
	}

	template<typename ErrHandler, bool ReuseAddr = true>
	requires std::invocable<ErrHandler, int>
	static TcpServerSocket instance(const InetAddress &addr,
					int backlog, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return instance<ErrHandler, ReuseAddr>
			(addr.saddrin(), backlog,
			 std::forward<ErrHandler>(handler));
	}

	template<bool ReuseAddr = true>
	static TcpServerSocket instance(const InetAddress &addr, int backlog)
	{
		return instance<ReuseAddr>(addr.saddrin(), backlog);
	}

	template<bool ReuseAddr = true>
	static TcpServerSocket instance(const struct sockaddr_in *addr,
					int backlog)
	{
		TcpServerSocket sock;

		sock.open();

		if (ReuseAddr)
			sock.setreuseaddr(true);

		sock.bind(addr);
		sock.listen(backlog);

		return sock;
	}
};


}


#endif
