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


#include <metasys/net/TcpBaseSocket.hxx>

#include <cassert>
#include <cerrno>

#include <metasys/net/InetAddress.hxx>
#include <metasys/sys/SystemException.hxx>


using metasys::InetAddress;
using metasys::SystemException;
using metasys::TcpBaseSocket;


void TcpBaseSocket::bind(const InetAddress &addr)
{
	bind(addr, [](int ret) {
		if (ret < 0) [[unlikely]]
			bindthrow();
	});
}

void TcpBaseSocket::bind(const struct sockaddr_in *addr)
{
	bind(addr, [](int ret) {
		if (ret < 0) [[unlikely]]
			bindthrow();
	});
}

void TcpBaseSocket::bindthrow()
{
	assert(errno != EBADF);
	assert(errno != EINVAL);
	assert(errno != ENOTSOCK);
	assert(errno != EFAULT);
	assert(errno != ENAMETOOLONG);

	SystemException::throwErrno();
}

InetAddress TcpBaseSocket::sockname() const
{
	InetAddress ret;

	sockname(&ret, [](int ret) {
		if (ret < 0) [[unlikely]]
			socknamethrow();
	});

	return ret;
}

void TcpBaseSocket::sockname(struct sockaddr_in *dest) const
{
	sockname(dest, [](int ret) {
		if (ret < 0) [[unlikely]]
			socknamethrow();
	});
}

void TcpBaseSocket::socknamethrow()
{
	assert(errno != EBADF);
	assert(errno != ENOTSOCK);
	assert(errno != EOPNOTSUPP);
	assert(errno != EINVAL);

	SystemException::throwErrno();
}

void TcpBaseSocket::setreuseaddr(bool allow) noexcept
{
	setreuseaddr(allow, [](int ret) {
		if (ret < 0) [[unlikely]]
			setsockoptthrow();
	});
}

void TcpBaseSocket::setsockoptthrow() noexcept
{
	assert(errno != EBADF);
	assert(errno != EFAULT);
	assert(errno != EINVAL);
	assert(errno != ENOPROTOOPT);
	assert(errno != ENOTSOCK);
}
