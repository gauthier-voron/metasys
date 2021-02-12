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


#include <metasys/net/TcpServerSocket.hxx>

#include <cassert>
#include <cerrno>

#include <metasys/net/InetAddress.hxx>
#include <metasys/net/TcpSocket.hxx>
#include <metasys/sys/SystemException.hxx>


using metasys::InetAddress;
using metasys::SystemException;
using metasys::TcpServerSocket;
using metasys::TcpSocket;


void TcpServerSocket::listen(int backlog)
{
	listen(backlog, [](int ret) {
		if (ret < 0) [[unlikely]]
			listenthrow();
	});
}

void TcpServerSocket::listenthrow()
{
	assert(errno != EBADF);
	assert(errno != ENOTSOCK);
	assert(errno != EOPNOTSUPP);

	SystemException::throwErrno();
}

TcpSocket TcpServerSocket::accept(InetAddress *from)
{
	return accept(from, [](int ret) {
		if (ret < 0) [[unlikely]]
			acceptthrow();
	});
}

TcpSocket TcpServerSocket::accept(struct sockaddr_in *from)
{
	return accept(from, [](int ret) {
		if (ret < 0) [[unlikely]]
			acceptthrow();
	});
}

TcpSocket TcpServerSocket::accept()
{
	return accept([](int ret) {
		if (ret < 0) [[unlikely]]
			acceptthrow();
	});
}

void TcpServerSocket::acceptthrow()
{
	assert(errno != EBADF);
	assert(errno != EFAULT);
	assert(errno != EINVAL);
	assert(errno != ENOTSOCK);
	assert(errno != EOPNOTSUPP);

	SystemException::throwErrno();
}

// TcpServerSocket TcpServerSocket::instance(const InetAddress &addr, int backlog)
// {
// 	return instance(addr.saddrin(), backlog);
// }

// TcpServerSocket TcpServerSocket::instance(const struct sockaddr_in *addr,
// 					  int backlog)
// {
// 	TcpServerSocket sock;

// 	sock.open();
// 	sock.bind(addr);
// 	sock.listen(backlog);

// 	return sock;
// }
