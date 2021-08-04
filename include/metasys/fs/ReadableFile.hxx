#ifndef _INCLUDE_METASYS_FS_READABLEFILE_HXX_
#define _INCLUDE_METASYS_FS_READABLEFILE_HXX_


#include <fcntl.h>

#include <cassert>
#include <cerrno>
#include <string>
#include <utility>

#include <metasys/fs/Openable.hxx>
#include <metasys/io/ReadableDescriptor.hxx>
#include <metasys/io/SeekableDescriptor.hxx>
#include <metasys/sys/ClosingDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


namespace details {


using ReadableFileBase =
	ReadableInterface
	<SeekableInterface
	 <ClosingDescriptor>>;


class ReadableFile : public details::ReadableFileBase
{
	static constexpr int _allowedFlags =
		  O_ASYNC
		| O_CLOEXEC
		| O_DIRECT
		| O_LARGEFILE
		| O_NOATIME
		| O_NOCTTY
		| O_NOFOLLOW
		| O_NONBLOCK;


 public:
	using details::ReadableFileBase::ReadableFileBase;


	template<typename ErrHandler>
	auto open(const char *pathname, int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);
		assert((flags & ~_allowedFlags) == 0);

		reset(::open(pathname, flags | O_RDONLY));

		return handler(value());
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static ReadableFile openinit(const char *pathname, int flags,
				     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int fd;

		assert((flags & ~_allowedFlags) == 0);

		fd = ::open(pathname, flags | O_RDONLY);

		handler(fd);

		return ReadableFile(fd);
	}

	[[noreturn]] static void throwopen()
	{
		assert(errno != EFAULT);
		assert(errno != ENAMETOOLONG);
		assert(errno != EOPNOTSUPP);
		assert(errno != EROFS);
		assert(errno != ETXTBSY);

		SystemException::throwErrno();
	}
};


}


class ReadableFile : public details::Openable<details::ReadableFile>
{
	using ThisBase = details::Openable<details::ReadableFile>;


 public:
	using ThisBase::ThisBase;

	ReadableFile(details::ReadableFile &&other)
		: ThisBase(std::move(other))
	{
	}
};



}


#endif
