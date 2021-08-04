#ifndef _TEST_UNIT_INCLUDE_FD_HXX_
#define _TEST_UNIT_INCLUDE_FD_HXX_


#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>


namespace test {


class Fd
{
 public:
	static bool isvalid(int fd) noexcept
	{
		return (::fcntl(fd, F_GETFD) >= 0);
	}

	static size_t size(int fd) noexcept
	{
		struct stat st;
		int ret [[maybe_unused]];

		ret = ::fstat(fd, &st);

		assert(ret == 0);

		return st.st_size;
	}

	static std::string slurp(int fd) noexcept
	{
		std::string ret;
		char buffer[128];
		ssize_t ssize;

		while ((ssize = ::read(fd, buffer, sizeof (buffer))) > 0)
			ret.append(buffer, (size_t) ssize);

		assert(ssize == 0);

		return ret;
	}
};


}


#endif
