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


#include <metasys/sys/FileDescriptor.hxx>

#include <cerrno>

#include <metasys/sys/SystemException.hxx>


using metasys::FileDescriptor;
using metasys::SystemException;


FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
	: _fd(other._fd)
{
	other._fd = -1;
}

FileDescriptor::~FileDescriptor()
{
	if (valid() == false)
		return;
	close([](auto){});
}

int FileDescriptor::reset(int fd) noexcept
{
	int tmp = _fd;

	_fd = fd;

	return tmp;
}

void FileDescriptor::close()
{
	close([](int ret) {
		if (ret < 0) [[unlikely]]
			closethrow();
	});
}

void FileDescriptor::closethrow()
{
	assert(errno != EBADF);

	SystemException::throwErrno();
}
