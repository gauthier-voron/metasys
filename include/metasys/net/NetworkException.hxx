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


#ifndef _INCLUDE_METASYS_NET_NETWORKEXCEPTION_HXX_
#define _INCLUDE_METASYS_NET_NETWORKEXCEPTION_HXX_


#include <metasys/sys/SystemException.hxx>


namespace metasys {


class NetworkException : public SystemException
{
 public:
	NetworkException() noexcept = default;
	NetworkException(const NetworkException &other) noexcept = default;
	NetworkException(NetworkException &&other) noexcept = default;

	NetworkException &operator=(const NetworkException &other) noexcept =
		default;
	NetworkException &operator=(NetworkException &&other) noexcept =
		default;

	const char *what() const noexcept override
	{
		return "generic network metasys exception";
	}
};


}


#endif
