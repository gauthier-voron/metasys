#include <metasys/fs/ReadWriteFile.hxx>

#include <cstdio>

#include <asmcmp.hxx>


using metasys::ReadWriteFile;


Model(OpenReadWriteFileSafe)
{
	int fd = ::open("/tmp/foo", O_RDWR);
	ssize_t ssize;

	if (fd < 0)
		throw 0;

	ssize = ::write(fd, "Hello World!\n", 13);

	if (ssize < 0)
		throw 0;

	::close(fd);
}
Test(OpenReadWriteFileSafe)
{
	ReadWriteFile::openinit("/tmp/foo").write("Hello World!\n", 13);
}

Model(OpenReadWriteFileUnsafe)
{
	int fd = ::open("/tmp/foo", O_RDWR);

	::write(fd, "Hello World!\n", 13);

	::close(fd);
}
Test(OpenReadWriteFileUnsafe)
{
	ReadWriteFile file = ReadWriteFile::openinit("/tmp/foo", [](auto){});

	file.write("Hello World!\n", 13, [](auto){});

	file.close([](auto){});
}

Model(OpenReadWriteFileFlagsSafe)
{
	int fd = ::open("/tmp/foo", O_RDWR | O_CLOEXEC | O_NOFOLLOW);
	ssize_t ssize;

	if (fd < 0)
		throw 0;

	ssize = ::write(fd, "Hello World!\n", 13);

	if (ssize < 0)
		throw 0;

	::close(fd);
}
Test(OpenReadWriteFileFlagsSafe)
{
	ReadWriteFile::openinit("/tmp/foo", O_CLOEXEC | O_NOFOLLOW)
		.write("Hello World!\n", 13);
}
