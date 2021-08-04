#ifndef _TEST_UNIT_INCLUDE_TEMPFILE_HXX_
#define _TEST_UNIT_INCLUDE_TEMPFILE_HXX_


#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>

#include <File.hxx>


namespace test {


class TempFile
{
	std::string  _path;


 public:
	TempFile() noexcept
		: _path("metasys-unit.XXXXXX")
	{
		int fd = ::mkstemp(_path.data());

		assert(fd >= 0);

		::close(fd);
	}

	explicit TempFile(const char *content) noexcept
		: _path("metasys-unit.XXXXXX")
	{
		int fd = ::mkstemp(_path.data());
		size_t done = 0, len = ::strlen(content);
		ssize_t ssize;

		assert(fd >= 0);

		while (done < len) {
			ssize = ::write(fd, content + done, len - done);

			assert(ssize > 0);

			done += (size_t) ssize;
		}

		::close(fd);
	}

	explicit TempFile(const std::string &content) noexcept
		: TempFile(content.c_str())
	{
	}


	~TempFile() noexcept
	{
		int ret [[maybe_unused]] = ::unlink(_path.c_str());

		assert(ret == 0);
	}


	const std::string &path() const noexcept
	{
		return _path;
	}

	std::string content() const noexcept
	{
		return File::slurp(path());
	}
};


}


#endif
