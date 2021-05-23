#ifndef _INCLUDE_METASYS_FS_STAT_HXX_
#define _INCLUDE_METASYS_FS_STAT_HXX_


#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <string>

#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


namespace detail {


struct NoFollow_t
{
};


}


class Stat : public stat
{
 public:
	constexpr Stat() noexcept = default;

	explicit constexpr Stat(const struct stat &buf) noexcept
		: stat(buf)
	{
	}

	template<typename ErrHandler>
	Stat(int fd, const char *path, int flags, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		scan(fd, path, flags, std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	Stat(const FileDescriptor &fd, const char *path, int flags,
	     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
		: Stat(fd.value(), path, flags,
		       std::forward<ErrHandler>(handler))
	{
	}

	template<typename ErrHandler>
	Stat(int fd, const std::string &path, int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
		: Stat(fd, path.c_str(), flags,
		       std::forward<ErrHandler>(handler))
	{
	}

	template<typename ErrHandler>
	Stat(const FileDescriptor &fd, const std::string &path, int flags,
	     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
		: Stat(fd.value(), path.c_str(), flags,
		       std::forward<ErrHandler>(handler))
	{
	}

	template<typename Fd, typename Path>
	Stat(Fd &&fd, Path &&path, int flags)
		: Stat(std::forward<Fd>(fd), std::forward<Path>(path),
				flags, [](int ret) {
			if (ret < 0) [[unlikely]]
				scanthrow();
		})
	{
	}

	template<typename Target, typename ErrHandler>
	Stat(Target &&target, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		scan(std::forward<Target>(target),
		     std::forward<ErrHandler>(handler));
	}

	template<typename Target>
	explicit Stat(Target &&target)
		: Stat(std::forward<Target>(target), [](int ret) {
			if (ret < 0) [[unlikely]]
				scanthrow();
		})
	{
	}

	static inline constexpr detail::NoFollow_t NoFollow {};

	template<typename Target, typename ErrHandler>
	Stat(detail::NoFollow_t, Target &&target, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		lscan(std::forward<Target>(target),
		      std::forward<ErrHandler>(handler));
	}

	template<typename Target>
	Stat(detail::NoFollow_t nf, Target &&target)
		: Stat(nf, std::forward<Target>(target), [](int ret) {
			if (ret < 0) [[unlikely]]
				scanthrow();
		})
	{
	}

	constexpr Stat(const Stat &other) noexcept = default;
	Stat(Stat &&other) noexcept = default;


	Stat &operator=(const Stat &other) noexcept
	{
		*this = other;
		return *this;
	}


	struct stat &value() noexcept
	{
		return *this;
	}

	const struct stat &value() const noexcept
	{
		return *this;
	}


	static constexpr int EMPTY_PATH       = AT_EMPTY_PATH;
	static constexpr int NO_AUTOMOUNT     = AT_NO_AUTOMOUNT;
	static constexpr int SYMLINK_NOFOLLOW = AT_SYMLINK_NOFOLLOW;


	template<typename ErrHandler>
	auto scan(int fd, const char *path, int flags, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return handler(::fstatat(fd, path, this, flags));
	}

	template<typename ErrHandler>
	auto scan(const FileDescriptor &fd, const char *path, int flags,
		  ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return scan(fd.value(), path, flags,
			    std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto scan(int fd, const std::string &path, int flags,
		  ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return scan(fd, path.c_str(), flags,
			    std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto scan(const FileDescriptor &fd, const std::string &path, int flags,
		  ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return scan(fd.value(), path.c_str(), flags,
			    std::forward<ErrHandler>(handler));
	}

	template<typename Fd, typename Path>
	void scan(Fd &&fd, Path &&path, int flags)
	{
		scan(std::forward<Fd>(fd), std::forward<Path>(path), flags,
		     [](int ret) {
			     if (ret < 0) [[unlikely]]
				     scanthrow();
		     });
	}


	template<typename ErrHandler>
	auto scan(const char *path, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return handler(::stat(path, this));
	}

	template<typename ErrHandler>
	auto scan(const std::string &path, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return scan(path.c_str(), std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto scan(int fd, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return handler(::fstat(fd, this));
	}

	template<typename ErrHandler>
	auto scan(const FileDescriptor &fd, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return scan(fd.value(), std::forward<ErrHandler>(handler));
	}

	template<typename Target>
	void scan(Target &&target)
	{
		scan(std::forward<Target>(target), [](int ret) {
			if (ret < 0) [[unlikely]]
				scanthrow();
		});
	}


	template<typename ErrHandler>
	auto lscan(const char *path, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return handler(::lstat(path, this));
	}

	template<typename ErrHandler>
	auto lscan(const std::string &path, ErrHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		return lscan(path.c_str(), std::forward<ErrHandler>(handler));
	}

	template<typename Path>
	void lscan(Path &&path)
	{
		lscan(std::forward<Path>(path), [](int ret) {
			if (ret < 0) [[unlikely]]
				scanthrow();
		});
	}

	static void scanthrow()
	{
		assert(errno != EBADF);
		assert(errno != EFAULT);
		assert(errno != EINVAL);
		assert(errno != ENAMETOOLONG);

		SystemException::throwErrno();
	}
};


}


#endif
