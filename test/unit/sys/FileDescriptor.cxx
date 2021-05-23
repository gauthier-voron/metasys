#include <metasys/sys/FileDescriptor.hxx>

#include <fcntl.h>

#include <gtest/gtest.h>


using metasys::FileDescriptor;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}

static inline int __get_new_fd()
{
	return ::dup(STDIN_FILENO);
}


TEST(FileDescriptor, Unassigned)
{
	FileDescriptor fd;

	EXPECT_FALSE(fd.valid());
}

TEST(FileDescriptor, ConstructFromSys)
{
	int sysfd = __get_new_fd();

	{
		FileDescriptor fd = FileDescriptor(sysfd);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(fd.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd.value()));
	}

	EXPECT_TRUE(__fd_is_valid(sysfd));

	::close(sysfd);
}

TEST(FileDescriptor, ConstructMove)
{
	int sysfd = __get_new_fd();

	{
		FileDescriptor fd0 = FileDescriptor(sysfd);
		FileDescriptor fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	::close(sysfd);
}

TEST(FileDescriptor, AssignMove)
{
	int sysfd = __get_new_fd();

	{
		FileDescriptor fd0 = FileDescriptor(sysfd);
		FileDescriptor fd1;

		fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	::close(sysfd);
}

TEST(FileDescriptor, Reassign)
{
	int sysfd0 = __get_new_fd();
	int sysfd1 = __get_new_fd();

	{
		FileDescriptor fd0 = FileDescriptor(sysfd0);
		FileDescriptor fd1 = FileDescriptor(sysfd1);

		fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd0);

		EXPECT_TRUE(__fd_is_valid(sysfd0));
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	::close(sysfd0);
	::close(sysfd1);
}

TEST(FileDescriptor, ReassignSame)
{
	int sysfd = __get_new_fd();

	{
		FileDescriptor fd = FileDescriptor(sysfd);

		fd = std::move(fd);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(fd.value(), sysfd);

		EXPECT_TRUE(__fd_is_valid(fd.value()));
	}

	::close(sysfd);
}

TEST(FileDescriptor, Reset)
{
	int sysfd0 = __get_new_fd();
	int sysfd1 = __get_new_fd();
	int ret;

	{
		FileDescriptor fd = FileDescriptor(sysfd0);

		ret = fd.reset(sysfd1);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(ret, sysfd0);
		EXPECT_EQ(fd.value(), sysfd1);

		EXPECT_TRUE(__fd_is_valid(sysfd0));
		EXPECT_TRUE(__fd_is_valid(sysfd1));
	}

	::close(sysfd0);
	::close(sysfd1);
}

TEST(FileDescriptor, Close)
{
	int sysfd = __get_new_fd();
	FileDescriptor fd = FileDescriptor(sysfd);

	fd.close();

	EXPECT_FALSE(fd.valid());
	EXPECT_FALSE(__fd_is_valid(sysfd));
}
