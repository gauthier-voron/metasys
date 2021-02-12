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


#include <metasys/net/TcpSocket.hxx>

#include <cassert>
#include <cerrno>

#include <metasys/net/InetAddress.hxx>
#include <metasys/sys/SystemException.hxx>


using metasys::InetAddress;
using metasys::SystemException;
using metasys::TcpSocket;


void TcpSocket::connect(const InetAddress &addr)
{
	connect(addr.saddrin());
}

void TcpSocket::connect(const struct sockaddr_in *addr)
{
	connect(addr, [](int ret) {
		if (ret < 0) [[unlikely]]
			connectthrow();
		});
}

void TcpSocket::connectthrow()
{
	assert(errno != EAFNOSUPPORT);
	assert(errno != EALREADY);
	assert(errno != EBADF);
	assert(errno != EFAULT);
	assert(errno != EISCONN);
	assert(errno != ENOTSOCK);
	assert(errno != EPROTOTYPE);

	SystemException::throwErrno();
}


void TcpSocket::disconnect()
{
	disconnect([](int ret) {
		if (ret < 0) [[unlikely]]
			disconnectthrow();
	});
}

void TcpSocket::disconnectthrow()
{
	assert(errno != EAFNOSUPPORT);
	assert(errno != EALREADY);
	assert(errno != EBADF);
	assert(errno != EFAULT);
	assert(errno != EISCONN);
	assert(errno != ENOTSOCK);
	assert(errno != EPROTOTYPE);

	SystemException::throwErrno();
}

size_t TcpSocket::read(void *dest, size_t len)
{
	return read(dest, len, [](ssize_t ret) {
		if (ret < 0) [[unlikely]]
			readthrow();
		return static_cast<size_t> (ret);
	});
}

void TcpSocket::readthrow()
{
	assert(errno != EBADF);
	assert(errno != EFAULT);
	assert(errno != EINVAL);

	SystemException::throwErrno();
}

size_t TcpSocket::write(const void *src, size_t len)
{
	return write(src, len, [](ssize_t ret) {
		if (ret < 0) [[unlikely]]
			writethrow();
		return static_cast<size_t> (ret);
	});
}

void TcpSocket::writethrow()
{
	assert(errno != EBADF);
	assert(errno != EDESTADDRREQ);
	assert(errno != EFAULT);
	assert(errno != EINVAL);

	SystemException::throwErrno();
}

TcpSocket TcpSocket::instance(const InetAddress &addr)
{
	return instance(addr.saddrin());
}

TcpSocket TcpSocket::instance(const struct sockaddr_in *addr)
{
	TcpSocket sock;

	sock.open();
	sock.connect(addr);

	return sock;
}
