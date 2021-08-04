#ifndef _INCLUDE_METASYS_IO_SEEKABLE_DESCRIPTOR_HXX_
#define _INCLUDE_METASYS_IO_SEEKABLE_DESCRIPTOR_HXX_


#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <utility>

#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


template<typename Descriptor>
class SeekableInterface : public Descriptor
{
 public:
	using Descriptor::Descriptor;

	SeekableInterface(SeekableInterface &&other) noexcept = default;


	SeekableInterface &operator=(SeekableInterface &&other) = default;


	template<typename ErrHandler>
	auto seek(off_t offset, int whence, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(Descriptor::valid());

		return handler(::lseek(Descriptor::value(), offset, whence));
	}

	template<typename ErrHandler>
	auto seek(off_t offset, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return seek(offset,SEEK_SET,std::forward<ErrHandler>(handler));
	}

	void seek(off_t offset, int whence = SEEK_SET)
	{
		seek(offset, whence, [](off_t ret) {
			if (ret == ((off_t) -1)) [[unlikely]]
				throwseek();
		});
	}

	static void throwseek()
	{
		assert(errno != EBADF);
		assert(errno != ESPIPE);

		SystemException::throwErrno();
	}
};

using SeekableDescriptor = SeekableInterface<FileDescriptor>;


}


#endif
