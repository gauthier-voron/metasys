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


#ifndef _INCLUDE_METASYS_SYS_SYSTEMEXCEPTION_HXX_
#define _INCLUDE_METASYS_SYS_SYSTEMEXCEPTION_HXX_


#include <metasys/sys/Exception.hxx>


namespace metasys {


class SystemException : public Exception
{
 public:
	SystemException() noexcept = default;
	SystemException(const SystemException &other) noexcept = default;
	SystemException(SystemException &&other) noexcept = default;

	SystemException &operator=(const SystemException &other) noexcept =
		default;
	SystemException &operator=(SystemException &&other) noexcept = default;

	const char *what() const noexcept override
	{
		return "generic system metasys exception";
	}

	[[noreturn]]
	static void throwErrno();

	[[noreturn]]
	static void throwErrno(int err);
};


}


#endif
