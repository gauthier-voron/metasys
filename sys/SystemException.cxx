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


#include <metasys/sys/SystemException.hxx>

#include <cerrno>
#include <cstdlib>

#include <metasys/sys/ErrnoException.hxx>


using metasys::ErrnoException;
using metasys::SystemException;


void SystemException::throwErrno()
{
	throwErrno(errno);
}

void SystemException::throwErrno(int err)
{
	switch (err) {
	case EACCES:
		throw ErrnoException<EACCES>();
	case EADDRINUSE:
		throw ErrnoException<EADDRINUSE>();
	case EADDRNOTAVAIL:
		throw ErrnoException<EADDRNOTAVAIL>();
	case EAGAIN:
		throw ErrnoException<EAGAIN>();
	case ECONNREFUSED:
		throw ErrnoException<ECONNREFUSED>();
	case EDESTADDRREQ:
		throw ErrnoException<EDESTADDRREQ>();
	case EDQUOT:
		throw ErrnoException<EDQUOT>();
	case EFBIG:
		throw ErrnoException<EFBIG>();
	case EINPROGRESS:
		throw ErrnoException<EINPROGRESS>();
	case EINTR:
		throw ErrnoException<EINTR>();
	case EINVAL:
		throw ErrnoException<EINVAL>();
	case EIO:
		throw ErrnoException<EIO>();
	case ELOOP:
		throw ErrnoException<ELOOP>();
	case EMFILE:
		throw ErrnoException<EMFILE>();
	case ENFILE:
		throw ErrnoException<ENFILE>();
	case ENOBUFS:
		throw ErrnoException<ENOBUFS>();
	case ENOENT:
		throw ErrnoException<ENOENT>();
	case ENOMEM:
		throw ErrnoException<ENOMEM>();
	case ENOSPC:
		throw ErrnoException<ENOSPC>();
	case ENOTDIR:
		throw ErrnoException<ENOTDIR>();
	case EOVERFLOW:
		throw ErrnoException<EOVERFLOW>();
	case EPERM:
		throw ErrnoException<EPERM>();
	case EPIPE:
		throw ErrnoException<EPIPE>();
	case EROFS:
		throw ErrnoException<EROFS>();
	case ESRCH:
		throw ErrnoException<ESRCH>();
	case ETIMEDOUT:
		throw ErrnoException<ETIMEDOUT>();
	default:
		::abort();
	}
}
