#include <metasys/fs/ReadWriteFile.hxx>

#include <fcntl.h>

#include <string>

#include <gtest/gtest.h>
#include <Fd.hxx>
#include <File.hxx>
#include <TempDirectory.hxx>
#include <TempFile.hxx>
#include <TempSymlink.hxx>

#include <metasys/sys/ErrnoException.hxx>


using metasys::ErrnoException;
using metasys::ReadWriteFile;
using std::string;
using test::Fd;
using test::File;
using test::TempDirectory;
using test::TempFile;
using test::TempSymlink;


TEST(ReadWriteFile, Unassigned)
{
	ReadWriteFile file;

	EXPECT_FALSE(file.valid());
}

TEST(ReadWriteFile, ConstructFromSys)
{
	TempFile tfile;
	int sysfd = File::open(tfile.path(), O_RDWR);

	{
		ReadWriteFile file = ReadWriteFile(sysfd);

		EXPECT_TRUE(file.valid());
		EXPECT_EQ(file.value(), sysfd);
		EXPECT_TRUE(Fd::isvalid(file.value()));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(ReadWriteFile, OpenInit)
{
	TempFile tfile;
	int sysfd;

	{
		ReadWriteFile file = ReadWriteFile::openinit(tfile.path());

		EXPECT_TRUE(file.valid());
		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(ReadWriteFile, Open)
{
	TempFile tfile;
	int sysfd;

	{
		ReadWriteFile file;

		EXPECT_FALSE(file.valid());

		file.open(tfile.path());

		EXPECT_TRUE(file.valid());
		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(ReadWriteFile, OpenInitNoFollow)
{
	TempFile tfile;
	TempSymlink tlink = TempSymlink(tfile.path());

	EXPECT_THROW(ReadWriteFile::openinit(tlink.path(), O_NOFOLLOW),
		     ErrnoException<ELOOP>);
}

TEST(ReadWriteFile, OpenInitNoFollowNoThrow)
{
	TempFile tfile;
	TempSymlink tlink = TempSymlink(tfile.path());
	ReadWriteFile file = ReadWriteFile::openinit(tlink.path(), O_NOFOLLOW,
						     [](auto){});

	EXPECT_FALSE(file.valid());
}

TEST(ReadWriteFile, OpenInitTrunc)
{
	TempFile tfile = TempFile("Some content\n");

	ReadWriteFile::openinit(tfile.path(), O_TRUNC);

	EXPECT_EQ(tfile.content(), "");
}

TEST(ReadWriteFile, CreateInitWrite)
{
	TempDirectory tdir;
	string path = tdir.path() + "/foo";
	int sysfd;

	{
		ReadWriteFile file = ReadWriteFile::createinit(path, 0644);

		EXPECT_TRUE(file.valid());

		file.write("Hello World!\n", 13);

		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
	EXPECT_EQ(File::slurp(path), "Hello World!\n");
}

TEST(ReadWriteFile, CreateWrite)
{
	TempDirectory tdir;
	string path = tdir.path() + "/foo";
	int sysfd;

	{
		ReadWriteFile file;

		EXPECT_FALSE(file.valid());

		file.create(path, 0644);
		file.write("Hello World!\n", 13);

		EXPECT_TRUE(file.valid());

		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
	EXPECT_EQ(File::slurp(path), "Hello World!\n");
}

TEST(ReadWriteFile, CreateInitExcl)
{
	TempFile tfile;

	EXPECT_THROW(ReadWriteFile::createinit(tfile.path(), O_EXCL, 0644),
		     ErrnoException<EEXIST>);
}

TEST(ReadWriteFile, CreateInitExclNoThrow)
{
	TempFile tfile;
	ReadWriteFile file = ReadWriteFile::createinit
		(tfile.path(), O_EXCL, 0644, [](auto){});

	EXPECT_FALSE(file.valid());
}

TEST(ReadWriteFile, CreateInitTmpWrite)
{
	TempDirectory tdir;
	int sysfd;

	{
		ReadWriteFile file = ReadWriteFile::createinit
			(tdir.path(), O_TMPFILE, 0644);

		EXPECT_TRUE(file.valid());
		EXPECT_TRUE(tdir.empty());

		file.write("Hello World!\n", 13);

		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));

		EXPECT_EQ(Fd::size(sysfd), 13);
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
	EXPECT_TRUE(tdir.empty());
}

TEST(ReadWriteFile, CreateInitTmpNoFollow)
{
	TempDirectory tdir;
	TempSymlink tlink = TempSymlink(tdir.path());

	ReadWriteFile::createinit(tdir.path(), O_TMPFILE | O_NOFOLLOW, 0644);

	EXPECT_THROW(ReadWriteFile::createinit
		     (tlink.path(), O_TMPFILE | O_NOFOLLOW, 0644),
		     ErrnoException<ENOTDIR>);
}
