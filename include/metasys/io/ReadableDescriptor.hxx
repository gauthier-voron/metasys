#ifndef _INCLUDE_METASYS_IO_READABLEDESCRIPTOR_HXX_
#define _INCLUDE_METASYS_IO_READABLEDESCRIPTOR_HXX_


#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <metasys/io/InputStream.hxx>
#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


template<typename Descriptor>
class ReadableInterface : public Descriptor
{
 public:
	using Descriptor::Descriptor;

	ReadableInterface(ReadableInterface &&other) noexcept = default;


	ReadableInterface &operator=(ReadableInterface &&other) = default;


	template<typename ErrorHandler>
	auto read(void *dest, size_t len, ErrorHandler &&handler)
		 noexcept (noexcept (handler(-1)))
	{
		assert(Descriptor::valid());

		return handler(::read(Descriptor::_fd, dest, len));
	}

	size_t read(void *dest, size_t len)
	{
		return read(dest, len, [](ssize_t ret) -> size_t {
			if (ret < 0) [[unlikely]]
				throwread();
			return (size_t) ret;
		});
	}

	static void throwread()
	{
		assert(errno != EBADF);
		assert(errno != EFAULT);
		assert(errno != EISDIR);

		SystemException::throwErrno();
	}
};

using ReadableDescriptor = ReadableInterface<FileDescriptor>;

static_assert (InputStream<ReadableDescriptor>);


constexpr ReadableDescriptor stdin = ReadableDescriptor(STDIN_FILENO);


}


#endif
