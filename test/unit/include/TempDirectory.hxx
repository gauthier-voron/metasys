#ifndef _TEST_UNIT_INCLUDE_TEMPDIRECTORY_HXX_
#define _TEST_UNIT_INCLUDE_TEMPDIRECTORY_HXX_


#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>
#include <cstring>
#include <string>
#include <vector>


namespace test {


class TempDirectory
{
	std::string  _path;


	void _removeContent(const char *pathname) noexcept
	{
		DIR *dh = ::opendir(pathname);
		struct dirent *dent;
		std::string npath;
		struct stat st;
		int ret;

		assert(dh != NULL);

		while ((dent = ::readdir(dh)) != NULL) {
			if (::strcmp(dent->d_name, ".") == 0)
				continue;
			if (::strcmp(dent->d_name, "..") == 0)
				continue;

			npath = pathname;
			npath.push_back('/');
			npath.append(dent->d_name);

			ret = ::lstat(npath.c_str(), &st);

			assert(ret == 0);

			switch (st.st_mode & S_IFMT) {
			case S_IFDIR:
				_removeContent(npath.c_str());
				ret = ::rmdir(npath.c_str());
				break;
			case S_IFIFO:
			case S_IFLNK:
			case S_IFREG:
			case S_IFSOCK:
				ret = ::unlink(npath.c_str());
				break;
			default:
				assert(0);
				break;
			}

			assert(ret == 0);
		}

		::closedir(dh);
	}


 public:
	TempDirectory() noexcept
		: _path("metasys-unit.XXXXXX")
	{
		char *ret [[maybe_unused]] = ::mkdtemp(_path.data());

		assert(ret != NULL);
	}

	~TempDirectory() noexcept
	{
		int ret [[maybe_unused]];

		_removeContent(_path.c_str());

		ret = ::rmdir(_path.c_str());

		assert(ret == 0);
	}

	const std::string &path() const noexcept
	{
		return _path;
	}

	std::vector<std::string> list() const noexcept
	{
		DIR *dh = ::opendir(_path.c_str());
		std::vector<std::string> ret;
		struct dirent *dent;

		assert(dh != NULL);

		while ((dent = ::readdir(dh)) != NULL) {
			if (::strcmp(dent->d_name, ".") == 0)
				continue;
			if (::strcmp(dent->d_name, "..") == 0)
				continue;

			ret.emplace_back(dent->d_name);
		}

		::closedir(dh);

		return ret;
	}

	bool empty() const noexcept
	{
		return list().empty();
	}
};


}


#endif
