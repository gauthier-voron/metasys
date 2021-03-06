#ifndef _INCLUDE_METASYS_IO_WRITABLEDESCRIPTOR_HXX_
#define _INCLUDE_METASYS_IO_WRITABLEDESCRIPTOR_HXX_


#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <metasys/io/OutputStream.hxx>
#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


template<typename Descriptor>
class WritableInterface : public Descriptor
{
 public:
	using Descriptor::Descriptor;

	WritableInterface(WritableInterface &&other) noexcept = default;


	WritableInterface &operator=(WritableInterface &&other) = default;


	template<typename ErrorHandler>
	[[gnu::always_inline]]
	auto write(const void *src, size_t len, ErrorHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		assert(Descriptor::valid());

		return handler(::write(Descriptor::_fd, src, len));
	}

	[[gnu::always_inline]]
	size_t write(const void *src, size_t len)
	{
		ssize_t ret;

		assert(Descriptor::valid());

	retry:
		ret = ::write(Descriptor::_fd, src, len);
		if (ret <= 0) [[unlikely]] {
			if ((errno == EAGAIN) || (errno == EINTR))
				goto retry;
			throwwrite();
		}

		return ((size_t) ret);
	}

	static void throwwrite()
	{
		assert(errno != EBADF);
		assert(errno != EDESTADDRREQ);
		assert(errno != EFAULT);
		assert(errno != EINVAL);
		assert(errno != EISDIR);

		SystemException::throwErrno();
	}
};

using WritableDescriptor = WritableInterface<FileDescriptor>;

static_assert (OutputStream<WritableDescriptor>);


extern WritableDescriptor stdout;
extern WritableDescriptor stderr;


}


#endif
