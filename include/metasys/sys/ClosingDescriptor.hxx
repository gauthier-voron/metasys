#ifndef _INCLUDE_METASYS_SYS_CLOSINGDESCRIPTOR_HXX_
#define _INCLUDE_METASYS_SYS_CLOSINGDESCRIPTOR_HXX_


#include <metasys/sys/FileDescriptor.hxx>


namespace metasys {


template<typename Descriptor>
class ClosingInterface : public Descriptor
{
 public:
	using Descriptor::Descriptor;

	template<typename ... Args>
	ClosingInterface(Args && ... args)
		noexcept (noexcept (Descriptor(std::forward<Args>(args) ...)))
		: Descriptor(std::forward<Args>(args) ...)
	{
	}

	~ClosingInterface() noexcept
	{
		if (Descriptor::valid())
			Descriptor::close([](auto){});
	}

	ClosingInterface &operator=(ClosingInterface &&other)
	{
		if (Descriptor::valid())
			if (other.value() != Descriptor::value()) [[likely]]
				Descriptor::close();

		Descriptor::reset(other.reset(-1));

		return *this;
	}
};

using ClosingDescriptor = ClosingInterface<FileDescriptor>;



}


#endif
