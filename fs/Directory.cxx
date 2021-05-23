#include <metasys/fs/Directory.hxx>

#include <dirent.h>
#include <sys/types.h>


using metasys::Directory;


Directory::Directory(Directory &&other) noexcept
	: _dh(other._dh)
{
	other._dh = nullptr;
}

Directory &Directory::operator=(Directory &&other) noexcept
{
	if (valid()) {
		if (other._dh != _dh) [[likely]]
			close();
	}

	_dh = other._dh;
	other._dh = nullptr;

	return *this;
}

DIR *Directory::reset(DIR *dh) noexcept
{
	DIR *tmp = _dh;

	_dh = dh;

	return tmp;
}
