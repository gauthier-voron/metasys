#ifndef _INCLUDE_METASYS_FS_DIRECTORY_HXX_
#define _INCLUDE_METASYS_FS_DIRECTORY_HXX_


#include <dirent.h>
#include <sys/types.h>

#include <cassert>
#include <cerrno>
#include <string>

#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


class Directory
{
 public:
	using Entry = struct dirent;


 private:
	DIR    *_dh;


 public:
	constexpr Directory() noexcept
		: _dh(nullptr)
	{
	}

	explicit constexpr Directory(DIR *dh) noexcept
		: _dh(dh)
	{
	}

	template<typename ErrHandler>
	Directory(const char *name, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
	{
		_dh = ::opendir(name);
		handler(_dh);
	}

	template<typename ErrHandler>
	Directory(const std::string &name, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
		: Directory(name.c_str(), std::forward<ErrHandler>(handler))
	{
	}

	template<typename ErrHandler>
	Directory(int fd, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
	{
		_dh = ::fdopendir(fd);
		handler(_dh);
	}

	template<typename ErrHandler>
	Directory(const FileDescriptor &fd, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
		: Directory(fd.value(), std::forward<ErrHandler>(handler))
	{
	}

	template<typename ErrHandler>
	Directory(FileDescriptor &&fd, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
		: Directory(fd.reset(), std::forward<ErrHandler>(handler))
	{
	}

	template<typename Target>
	explicit Directory(Target &&target)
		: Directory(std::forward<Target>(target), [](DIR *ret) {
			if (ret == NULL) [[unlikely]]
				openthrow();
		})
	{
	}


	Directory(const Directory &other) = delete;
	Directory(Directory &&other) noexcept;

	~Directory() noexcept
	{
		if (valid() == false)
			return;
		close();
	}

	Directory &operator=(const Directory &other) = delete;
	Directory &operator=(Directory &&other) noexcept;


	constexpr DIR *value() const noexcept
	{
		return _dh;
	}

	constexpr bool valid() const noexcept
	{
		return (_dh != nullptr);
	}

	DIR *reset(DIR *dh) noexcept;


	template<typename ErrHandler>
	auto open(const char *name, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
	{
		assert(valid() == false);

		_dh = ::opendir(name);

		return handler(_dh);
	}

	template<typename ErrHandler>
	auto open(const std::string &name, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
	{
		return open(name.c_str(), std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto open(int fd, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
	{
		assert(valid() == false);

		_dh = ::fdopendir(fd);

		return handler(_dh);
	}

	template<typename ErrHandler>
	auto open(const FileDescriptor &fd, ErrHandler &&handler)
		noexcept (noexcept (handler(NULL)))
	{
		return open(fd.value(), std::forward<ErrHandler>(handler));
	}

	template<typename Target>
	void open(Target &&target)
	{
		open(std::forward<Target>(target), [](DIR *ret) {
			if (ret == NULL) [[unlikely]]
				openthrow();
		});
	}

	static void openthrow()
	{
		assert(errno != EBADF);

		SystemException::throwErrno();
	}


	void close() noexcept
	{
		[[maybe_unused]] int ret;

		assert(valid());

		ret = ::closedir(_dh);

		assert(ret == 0);  // The only reason is EBADF
	}


	Entry *read() noexcept
	{
		assert(valid());

		// The only possible error is EBADF
		return ::readdir(_dh);
	}
};


}


#endif
