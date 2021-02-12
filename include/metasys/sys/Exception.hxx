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


#ifndef _INCLUDE_METASYS_SYS_EXCEPTION_HXX_
#define _INCLUDE_METASYS_SYS_EXCEPTION_HXX_


#include <exception>


namespace metasys {


class Exception : public std::exception
{
 public:
	Exception() noexcept = default;
	Exception(const Exception &other) noexcept = default;
	Exception(Exception &&other) noexcept = default;

	Exception &operator=(const Exception &other) noexcept = default;
	Exception &operator=(Exception &&other) noexcept = default;

	const char *what() const noexcept override
	{
		return "unknown metasys exception";
	}
};


}


#endif
