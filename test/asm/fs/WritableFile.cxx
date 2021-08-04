#include <metasys/fs/WritableFile.hxx>

#include <cstdio>

#include <asmcmp.hxx>


using metasys::WritableFile;


Model(OpenWritableFileSafe)
{
	int fd = ::open("/tmp/foo", O_WRONLY);
	ssize_t ssize;

	if (fd < 0)
		throw 0;

	ssize = ::write(fd, "Hello World!\n", 13);

	if (ssize < 0)
		throw 0;

	::close(fd);
}
Test(OpenWritableFileSafe)
{
	WritableFile::openinit("/tmp/foo").write("Hello World!\n", 13);
}

Model(OpenWritableFileUnsafe)
{
	int fd = ::open("/tmp/foo", O_WRONLY);

	::write(fd, "Hello World!\n", 13);

	::close(fd);
}
Test(OpenWritableFileUnsafe)
{
	WritableFile file = WritableFile::openinit("/tmp/foo", [](auto){});

	file.write("Hello World!\n", 13, [](auto){});

	file.close([](auto){});
}

Model(OpenWritableFileFlagsSafe)
{
	int fd = ::open("/tmp/foo", O_WRONLY | O_CLOEXEC | O_NOFOLLOW);
	ssize_t ssize;

	if (fd < 0)
		throw 0;

	ssize = ::write(fd, "Hello World!\n", 13);

	if (ssize < 0)
		throw 0;

	::close(fd);
}
Test(OpenWritableFileFlagsSafe)
{
	WritableFile::openinit("/tmp/foo", O_CLOEXEC | O_NOFOLLOW)
		.write("Hello World!\n", 13);
}
