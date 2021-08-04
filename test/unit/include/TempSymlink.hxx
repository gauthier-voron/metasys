#ifndef _TEST_UNIT_INCLUDE_TEMPSYMLINK_HXX_
#define _TEST_UNIT_INCLUDE_TEMPSYMLINK_HXX_


#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <string>


namespace test {


class TempSymlink
{
	std::string  _path;


 public:
	explicit TempSymlink(const char *target) noexcept
		: _path("metasys-unit.XXXXXX")
	{
		int fd = ::mkstemp(_path.data());
		std::string buffer = std::string(target);
		ssize_t ssize;
		int ret;

		assert(fd >= 0);

		::close(fd);
		::unlink(_path.c_str());

		ret = ::symlink(target, _path.c_str());

		assert(ret == 0);

		ssize = ::readlink(_path.c_str(), buffer.data(),
				   buffer.length());

		assert(ssize > 0);
		assert(((size_t) ssize) == buffer.length());
		assert(buffer == target);
	}

	explicit TempSymlink(const std::string &target) noexcept
		: TempSymlink(target.c_str())
	{
	}

	~TempSymlink() noexcept
	{
		int ret [[maybe_unused]] = ::unlink(_path.c_str());

		assert(ret == 0);
	}

	const std::string &path() const noexcept
	{
		return _path;
	}
};


}


#endif
