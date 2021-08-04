#ifndef _TEST_UNIT_INCLUDE_FILE_HXX_
#define _TEST_UNIT_INCLUDE_FILE_HXX_


#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <string>

#include <Fd.hxx>


namespace test {


class File
{
 public:
	static int open(const char *pathname, int flags, mode_t mode = 0)
		noexcept
	{
		int ret = ::open(pathname, flags, mode);

		assert(ret >= 0);

		return ret;
	}

	static int open(const std::string &pathname, int flags,
			mode_t mode = 0) noexcept
	{
		return open(pathname.c_str(), flags, mode);
	}


	static std::string slurp(const char *pathname) noexcept
	{
		std::string ret;
		int fd;

		fd = ::open(pathname, O_RDONLY);

		assert(fd >= 0);

		ret = Fd::slurp(fd);

		::close(fd);

		return ret;
	}

	static std::string slurp(const std::string &pathname) noexcept
	{
		return slurp(pathname.c_str());
	}
};


}


#endif
