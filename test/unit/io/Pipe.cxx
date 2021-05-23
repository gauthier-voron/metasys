#include <metasys/io/Pipe.hxx>

#include <fcntl.h>

#include <cstring>

#include <gtest/gtest.h>


using metasys::Pipe;
using std::string;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}


TEST(Pipe, Unassigned)
{
	Pipe pipe;

	EXPECT_FALSE(pipe.rend().valid());
	EXPECT_FALSE(pipe.wend().valid());
}

TEST(Pipe, Open)
{
	int rfd, wfd;

	{
		Pipe pipe;

		pipe.open();

		EXPECT_TRUE(pipe.rend().valid());
		EXPECT_TRUE(pipe.wend().valid());

		rfd = pipe.rend().value();
		wfd = pipe.wend().value();

		EXPECT_TRUE(__fd_is_valid(rfd));
		EXPECT_TRUE(__fd_is_valid(wfd));
	}

	EXPECT_FALSE(__fd_is_valid(rfd));
	EXPECT_FALSE(__fd_is_valid(wfd));
}

TEST(Pipe, OpenInit)
{
	int rfd, wfd;

	{
		Pipe pipe = Pipe::openinit();

		EXPECT_TRUE(pipe.rend().valid());
		EXPECT_TRUE(pipe.wend().valid());

		rfd = pipe.rend().value();
		wfd = pipe.wend().value();

		EXPECT_TRUE(__fd_is_valid(rfd));
		EXPECT_TRUE(__fd_is_valid(wfd));
	}

	EXPECT_FALSE(__fd_is_valid(rfd));
	EXPECT_FALSE(__fd_is_valid(wfd));
}

TEST(Pipe, WriteRead)
{
	Pipe pipe = Pipe::openinit();
	char buf[16];

	EXPECT_EQ(pipe.wend().write("Hello", 6), 6);

	EXPECT_EQ(pipe.rend().read(buf, sizeof (buf)), 6);

	EXPECT_EQ(::memcmp(buf, "Hello", 6), 0);
}

TEST(Pipe, MoveRead)
{
	int rfd;

	{
		Pipe pipe = Pipe::openinit();
		Pipe::Reader reader;

		EXPECT_TRUE(pipe.rend().valid());
		EXPECT_FALSE(reader.valid());

		rfd = pipe.rend().value();

		EXPECT_TRUE(__fd_is_valid(rfd));

		reader = pipe.rmove();

		EXPECT_FALSE(pipe.rend().valid());
		EXPECT_TRUE(reader.valid());

		EXPECT_EQ(reader.value(), rfd);
	}

	EXPECT_FALSE(__fd_is_valid(rfd));
}

TEST(Pipe, MoveWrite)
{
	int wfd;

	{
		Pipe pipe = Pipe::openinit();
		Pipe::Writer writer;

		EXPECT_TRUE(pipe.wend().valid());
		EXPECT_FALSE(writer.valid());

		wfd = pipe.wend().value();

		EXPECT_TRUE(__fd_is_valid(wfd));

		writer = pipe.wmove();

		EXPECT_FALSE(pipe.wend().valid());
		EXPECT_TRUE(writer.valid());

		EXPECT_EQ(writer.value(), wfd);
	}

	EXPECT_FALSE(__fd_is_valid(wfd));
}

TEST(Pipe, WriteRmoveRead)
{
	Pipe pipe = Pipe::openinit();
	Pipe::Reader reader;
	char buf[16];

	EXPECT_EQ(pipe.wend().write("Hello", 6), 6);

	reader = pipe.rmove();

	EXPECT_EQ(reader.read(buf, sizeof (buf)), 6);

	EXPECT_EQ(::memcmp(buf, "Hello", 6), 0);
}

TEST(Pipe, RmoveWriteRead)
{
	Pipe pipe = Pipe::openinit();
	Pipe::Reader reader = pipe.rmove();
	char buf[16];

	EXPECT_EQ(pipe.wend().write("Hello", 6), 6);

	EXPECT_EQ(reader.read(buf, sizeof (buf)), 6);

	EXPECT_EQ(::memcmp(buf, "Hello", 6), 0);
}

TEST(Pipe, WmoveWriteRead)
{
	Pipe pipe = Pipe::openinit();
	Pipe::Writer writer = pipe.wmove();
	char buf[16];

	EXPECT_EQ(writer.write("Hello", 6), 6);

	EXPECT_EQ(pipe.rend().read(buf, sizeof (buf)), 6);

	EXPECT_EQ(::memcmp(buf, "Hello", 6), 0);
}

TEST(Pipe, MoveWriteRead)
{
	Pipe pipe = Pipe::openinit();
	Pipe::Reader reader = pipe.rmove();
	Pipe::Writer writer = pipe.wmove();
	char buf[16];

	EXPECT_EQ(writer.write("Hello", 6), 6);

	EXPECT_EQ(reader.read(buf, sizeof (buf)), 6);

	EXPECT_EQ(::memcmp(buf, "Hello", 6), 0);
}
