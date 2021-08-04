#include <metasys/fs/WritableFile.hxx>

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
using metasys::WritableFile;
using std::string;
using test::Fd;
using test::File;
using test::TempDirectory;
using test::TempFile;
using test::TempSymlink;


TEST(WritableFile, Unassigned)
{
	WritableFile file;

	EXPECT_FALSE(file.valid());
}

TEST(WritableFile, ConstructFromSys)
{
	TempFile tfile;
	int sysfd = File::open(tfile.path(), O_WRONLY);

	{
		WritableFile file = WritableFile(sysfd);

		EXPECT_TRUE(file.valid());
		EXPECT_EQ(file.value(), sysfd);
		EXPECT_TRUE(Fd::isvalid(file.value()));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(WritableFile, OpenInit)
{
	TempFile tfile;
	int sysfd;

	{
		WritableFile file = WritableFile::openinit(tfile.path());

		EXPECT_TRUE(file.valid());
		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(WritableFile, Open)
{
	TempFile tfile;
	int sysfd;

	{
		WritableFile file;

		EXPECT_FALSE(file.valid());

		file.open(tfile.path());

		EXPECT_TRUE(file.valid());
		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
}

TEST(WritableFile, OpenInitWrite)
{
	TempFile tfile = TempFile("Hello there!\n");
	WritableFile file = WritableFile::openinit(tfile.path());
	size_t len;

	EXPECT_TRUE(file.valid());

	len = file.write("Greetings!\n", 11);

	EXPECT_EQ(len, 11);

	EXPECT_EQ(tfile.content(), "Greetings!\n!\n");
}

TEST(WritableFile, OpenInitNoFollow)
{
	TempFile tfile;
	TempSymlink tlink = TempSymlink(tfile.path());

	EXPECT_THROW(WritableFile::openinit(tlink.path(), O_NOFOLLOW),
		     ErrnoException<ELOOP>);
}

TEST(WritableFile, OpenInitNoFollowNoThrow)
{
	TempFile tfile;
	TempSymlink tlink = TempSymlink(tfile.path());
	WritableFile file = WritableFile::openinit(tlink.path(), O_NOFOLLOW,
						   [](auto){});

	EXPECT_FALSE(file.valid());
}

TEST(WritableFile, OpenInitTrunc)
{
	TempFile tfile = TempFile("Some content\n");

	WritableFile::openinit(tfile.path(), O_TRUNC);

	EXPECT_EQ(tfile.content(), "");
}

TEST(WritableFile, CreateInitWrite)
{
	TempDirectory tdir;
	string path = tdir.path() + "/foo";
	int sysfd;

	{
		WritableFile file = WritableFile::createinit(path, 0644);

		EXPECT_TRUE(file.valid());

		file.write("Hello World!\n", 13);

		sysfd = file.value();
		EXPECT_TRUE(Fd::isvalid(sysfd));
	}

	EXPECT_FALSE(Fd::isvalid(sysfd));
	EXPECT_EQ(File::slurp(path), "Hello World!\n");
}

TEST(WritableFile, CreateWrite)
{
	TempDirectory tdir;
	string path = tdir.path() + "/foo";
	int sysfd;

	{
		WritableFile file;

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

TEST(WritableFile, CreateInitExcl)
{
	TempFile tfile;

	EXPECT_THROW(WritableFile::createinit(tfile.path(), O_EXCL, 0644),
		     ErrnoException<EEXIST>);
}

TEST(WritableFile, CreateInitExclNoThrow)
{
	TempFile tfile;
	WritableFile file = WritableFile::createinit
		(tfile.path(), O_EXCL, 0644, [](auto){});

	EXPECT_FALSE(file.valid());
}

TEST(WritableFile, CreateInitTmpWrite)
{
	TempDirectory tdir;
	int sysfd;

	{
		WritableFile file = WritableFile::createinit
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

TEST(WritableFile, CreateInitTmpNoFollow)
{
	TempDirectory tdir;
	TempSymlink tlink = TempSymlink(tdir.path());

	WritableFile::createinit(tdir.path(), O_TMPFILE | O_NOFOLLOW, 0644);

	EXPECT_THROW(WritableFile::createinit
		     (tlink.path(), O_TMPFILE | O_NOFOLLOW, 0644),
		     ErrnoException<ENOTDIR>);
}
