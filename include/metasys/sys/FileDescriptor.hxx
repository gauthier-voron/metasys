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


#ifndef _INCLUDE_METASYS_SYS_DESCRIPTOR_HXX_
#define _INCLUDE_METASYS_SYS_DESCRIPTOR_HXX_


#include <unistd.h>

#include <cassert>
#include <cerrno>

#include <metasys/sys/SystemException.hxx>


namespace metasys {


class FileDescriptor
{
 protected:
	int  _fd;


 public:
	constexpr FileDescriptor() noexcept
		: _fd(-1)
	{
	}

	explicit constexpr FileDescriptor(int fd) noexcept
		: _fd(fd)
	{
	}

	FileDescriptor(const FileDescriptor &other) noexcept = default;

	FileDescriptor(FileDescriptor &&other) noexcept
		: _fd(other._fd)
	{
		other._fd = -1;
	}

	~FileDescriptor() = default;

	FileDescriptor &operator=(const FileDescriptor &other) noexcept =
		default;

	FileDescriptor &operator=(FileDescriptor &&other)
	{
		int tmp = other._fd;

		other._fd = -1;
		_fd = tmp;

		return *this;
	}

	constexpr int value() const noexcept
	{
		return _fd;
	}

	constexpr bool valid() const noexcept
	{
		return (_fd >= 0);
	}

	int reset(int fd = -1) noexcept
	{
		int tmp = _fd;

		_fd = fd;

		return tmp;
	}

	template<typename ErrHandler>
	auto close(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		int tmp;

		assert(valid());

		tmp = _fd;
		_fd = -1;

		return handler(::close(tmp));
	}

	void close()
	{
		close([](int ret) {
			if (ret < 0) [[unlikely]]
				closethrow();
		});
	}

	static void closethrow()
	{
		assert(errno != EBADF);

		SystemException::throwErrno();
	}
};


}


#endif
