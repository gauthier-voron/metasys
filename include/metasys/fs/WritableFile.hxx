#ifndef _INCLUDE_METASYS_FS_WRITABLEFILE_HXX_
#define _INCLUDE_METASYS_FS_WRITABLEFILE_HXX_


#include <fcntl.h>

#include <cassert>
#include <cerrno>
#include <utility>

#include <metasys/fs/Creatable.hxx>
#include <metasys/fs/Openable.hxx>
#include <metasys/io/SeekableDescriptor.hxx>
#include <metasys/io/WritableDescriptor.hxx>
#include <metasys/sys/ClosingDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


namespace details {


using WritableFileBase =
	WritableInterface
	<SeekableInterface
	 <ClosingDescriptor>>;


class WritableFile : public WritableFileBase
{
	static constexpr int _openAllowedFlags =
		  O_APPEND
		| O_ASYNC
		| O_CLOEXEC
		| O_DIRECT
		| O_DSYNC
		| O_LARGEFILE
		| O_NOFOLLOW
		| O_NONBLOCK
		| O_SYNC
		| O_TRUNC;

	static constexpr int _createAllowedFlags =
		  _openAllowedFlags
		| O_EXCL;

	static constexpr int _tmpfileAllowedFlags =
		  O_ASYNC
		| O_CLOEXEC
		| O_DIRECT
		| O_DSYNC
		| O_LARGEFILE
		| O_NOFOLLOW
		| O_TMPFILE
		| O_SYNC;


 public:
	using WritableFileBase::WritableFileBase;


	template<typename ErrHandler>
	auto open(const char *pathname, int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);
		assert((flags & ~_openAllowedFlags) == 0);

		reset(::open(pathname, flags | O_WRONLY));

		return handler(value());
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static WritableFile openinit(const char *pathname, int flags,
				     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int fd;

		assert((flags & ~_openAllowedFlags) == 0);

		fd = ::open(pathname, flags | O_WRONLY);

		handler(fd);

		return WritableFile(fd);
	}

	[[noreturn]] static void throwopen()
	{
		assert(errno != EFAULT);
		assert(errno != ENAMETOOLONG);
		assert(errno != EOPNOTSUPP);
		assert(errno != ETXTBSY);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto create(const char *pathname, int flags, mode_t mode,
		    ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);

		if ((flags & O_TMPFILE) != 0) {
			assert((flags & ~_tmpfileAllowedFlags) == 0);
			reset(::open(pathname, flags | O_WRONLY, mode));
		} else {
			assert((flags & ~_createAllowedFlags) == 0);
			reset(::open(pathname, flags|O_WRONLY|O_CREAT, mode));
		}

		return handler(value());
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static WritableFile createinit(const char *pathname, int flags,
				       mode_t mode, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int fd;

		if ((flags & O_TMPFILE) != 0) {
			assert((flags & ~_tmpfileAllowedFlags) == 0);
			fd = ::open(pathname, flags | O_WRONLY, mode);
		} else {
			assert((flags & ~_createAllowedFlags) == 0);
			fd = ::open(pathname, flags | O_WRONLY|O_CREAT, mode);
		}

		handler(fd);

		return WritableFile(fd);
	}

	[[noreturn]] static void throwcreate()
	{
		throwopen();
	}
};


using WritableFileExtendedBase =
	Creatable
	<Openable
	 <WritableFile>>;


}


class WritableFile : public details::WritableFileExtendedBase
{
	using ThisBase = details::WritableFileExtendedBase;


 public:
	using ThisBase::ThisBase;

	WritableFile(details::WritableFile &&other)
		: ThisBase(std::move(other))
	{
	}
};


}


#endif
