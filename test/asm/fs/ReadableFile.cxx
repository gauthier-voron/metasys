#include <metasys/fs/ReadableFile.hxx>

#include <cstdio>

#include <asmcmp.hxx>


using metasys::ReadableFile;


Model(OpenReadableFileSafe)
{
	int fd = ::open("/etc/passwd", O_RDONLY);
	char buf[128];
	ssize_t ssize;

	if (fd < 0)
		throw 0;

	ssize = ::read(fd, buf, sizeof(buf) - 1);

	if (ssize < 0)
		throw 0;

	buf[ssize] = '\0';

	printf("%s\n", buf);

	::close(fd);
}
Test(OpenReadableFileSafe)
{
	ReadableFile file = ReadableFile::openinit("/etc/passwd");
	char buf[128];
	size_t size;

	size = file.read(buf, sizeof (buf) - 1);
	buf[size] = '\0';

	printf("%s\n", buf);
}

Model(OpenReadableFileUnsafe)
{
	int fd = ::open("/etc/passwd", O_RDONLY);
	char buf[128];
	ssize_t ssize;

	ssize = ::read(fd, buf, sizeof(buf) - 1);
	buf[ssize] = '\0';

	printf("%s\n", buf);

	::close(fd);
}
Test(OpenReadableFileUnsafe)
{
	ReadableFile file = ReadableFile::openinit("/etc/passwd", [](auto){});
	char buf[128];
	ssize_t ssize;

	ssize = file.read(buf, sizeof (buf) - 1, [](auto r){ return r; });
	buf[ssize] = '\0';

	printf("%s\n", buf);

	file.close([](auto){});
}

Model(OpenReadableFileFlagsSafe)
{
	int fd = ::open("/etc/passwd", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	char buf[128];
	ssize_t ssize;

	if (fd < 0)
		throw 0;

	ssize = ::read(fd, buf, sizeof(buf) - 1);

	if (ssize < 0)
		throw 0;

	buf[ssize] = '\0';

	printf("%s\n", buf);

	::close(fd);
}
Test(OpenReadableFileFlagsSafe)
{
	ReadableFile file = ReadableFile::openinit
		("/etc/passwd", O_CLOEXEC | O_NOFOLLOW);
	char buf[128];
	size_t size;

	size = file.read(buf, sizeof (buf) - 1);
	buf[size] = '\0';

	printf("%s\n", buf);
}
