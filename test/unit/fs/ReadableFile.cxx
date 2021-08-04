#include <metasys/fs/ReadableFile.hxx>

#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <gtest/gtest.h>
#include <Fd.hxx>
#include <File.hxx>
#include <TempFile.hxx>
#include <TempSymlink.hxx>

#include <metasys/sys/ErrnoException.hxx>


using metasys::ErrnoException;
using metasys::ReadableFile;
using std::string;
using test::Fd;
using test::File;
using test::TempFile;
using test::TempSymlink;


TEST(ReadableFile, Unassigned)
{
	ReadableFile file;

	EXPECT_FALSE(file.valid());
}

TEST(ReadableFile, ConstructFromSys)
{
	TempFile tfile;
	int sysfd = File::open(tfile.path(), O_RDONLY);

	{
		ReadableFile file = ReadableFile(sysfd);

		EXPECT_TRUE(file.valid());
		EXPECT_EQ(file.value(), sysfd);
		EXPECT_TRUE(Fd::isvalid(file.value()));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(ReadableFile, OpenInit)
{
	TempFile tfile;
	int sysfd;

	{
		ReadableFile file = ReadableFile::openinit(tfile.path());

		EXPECT_TRUE(file.valid());
		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(ReadableFile, Open)
{
	TempFile tfile;
	int sysfd;

	{
		ReadableFile file;

		EXPECT_FALSE(file.valid());

		file.open(tfile.path());

		EXPECT_TRUE(file.valid());
		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(ReadableFile, OpenInitRead)
{
	string content = "Hello there!\n";
	TempFile tfile = TempFile(content);
	ReadableFile file = ReadableFile::openinit(tfile.path());
	char buffer[128];
	size_t len;

	EXPECT_TRUE(file.valid());

	len = file.read(buffer, sizeof (buffer) - 1);

	EXPECT_EQ(len, content.length());

	buffer[len] = '\0';

	EXPECT_EQ(buffer, content);
}

TEST(ReadableFile, OpenInitNoFollow)
{
	TempFile tfile;
	TempSymlink tlink = TempSymlink(tfile.path());

	EXPECT_THROW(ReadableFile::openinit(tlink.path(), O_NOFOLLOW),
		     ErrnoException<ELOOP>);
}

TEST(ReadableFile, OpenInitNoFollowNoThrow)
{
	TempFile tfile = TempFile("");
	TempSymlink tlink = TempSymlink(tfile.path());
	ReadableFile file = ReadableFile::openinit(tlink.path(), O_NOFOLLOW,
						   [](auto){});

	EXPECT_FALSE(file.valid());
}
