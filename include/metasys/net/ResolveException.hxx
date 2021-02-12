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


#ifndef _INCLUDE_METASYS_NET_RESOLVEEXCEPTION_HXX_
#define _INCLUDE_METASYS_NET_RESOLVEEXCEPTION_HXX_


#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <metasys/net/NetworkException.hxx>


namespace metasys {


template<int GaiErr>
class ResolveException : public NetworkException
{
 public:
	ResolveException() noexcept = default;
	ResolveException(const ResolveException &other) noexcept = default;
	ResolveException(ResolveException &&other) noexcept = default;

	ResolveException &operator=(const ResolveException &other) noexcept =
		default;
	ResolveException &operator=(ResolveException &&other) noexcept =
		default;

	const char *what() const noexcept override
	{
		return gai_strerror(GaiErr);
	}
};


}


#endif
