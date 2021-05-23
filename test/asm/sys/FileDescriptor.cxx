#include <metasys/sys/FileDescriptor.hxx>

#include <unistd.h>

#include <asmcmp.hxx>


using metasys::FileDescriptor;


Model(BuildCloseUnsafe)
{
	int fd = ::dup(STDIN_FILENO);

	asm volatile ("nop");

	::close(fd);
}
Test(BuildCloseUnsafe)
{
	FileDescriptor fd = FileDescriptor(::dup(STDIN_FILENO));

	asm volatile ("nop");

	fd.close([](auto){});
}


Model(BuildCloseSafe)
{
	int fd = ::dup(STDIN_FILENO);

	asm volatile ("nop");

	if (::close(fd) != 0)
		throw 0;
}
Test(BuildCloseSafe)
{
	FileDescriptor fd = FileDescriptor(::dup(STDIN_FILENO));

	asm volatile ("nop");

	fd.close();
}
