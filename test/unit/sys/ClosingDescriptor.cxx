#include <metasys/sys/ClosingDescriptor.hxx>

#include <fcntl.h>

#include <gtest/gtest.h>

#include <metasys/sys/FileDescriptor.hxx>


using metasys::ClosingDescriptor;
using metasys::FileDescriptor;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}

static inline int __get_new_fd()
{
	return ::dup(STDIN_FILENO);
}


TEST(ClosingDescriptor, Unassigned)
{
	ClosingDescriptor fd;

	EXPECT_FALSE(fd.valid());
}

TEST(ClosingDescriptor, ConstructFromSys)
{
	int sysfd = __get_new_fd();

	{
		ClosingDescriptor fd = ClosingDescriptor(sysfd);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(fd.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(ClosingDescriptor, ConstructMove)
{
	int sysfd = __get_new_fd();

	{
		ClosingDescriptor fd0 = ClosingDescriptor(sysfd);
		ClosingDescriptor fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(ClosingDescriptor, ConstructFromBase)
{
	int sysfd = __get_new_fd();

	{
		FileDescriptor fd0 = FileDescriptor(sysfd);
		ClosingDescriptor fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(ClosingDescriptor, AssignMove)
{
	int sysfd = __get_new_fd();

	{
		ClosingDescriptor fd0 = ClosingDescriptor(sysfd);
		ClosingDescriptor fd1;

		fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(ClosingDescriptor, Reassign)
{
	int sysfd0 = __get_new_fd();
	int sysfd1 = __get_new_fd();

	{
		ClosingDescriptor fd0 = ClosingDescriptor(sysfd0);
		ClosingDescriptor fd1 = ClosingDescriptor(sysfd1);

		fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());
		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd0);

		EXPECT_FALSE(__fd_is_valid(sysfd1));
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd0));
}

TEST(ClosingDescriptor, ReassignSame)
{
	int sysfd = __get_new_fd();

	{
		ClosingDescriptor fd = ClosingDescriptor(sysfd);

		fd = std::move(fd);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(fd.value(), sysfd);

		EXPECT_TRUE(__fd_is_valid(fd.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(ClosingDescriptor, ReassignCopy)
{
	int sysfd = __get_new_fd();

	{
		ClosingDescriptor fd0 = ClosingDescriptor(sysfd);
		ClosingDescriptor fd1 = ClosingDescriptor(sysfd);

		fd0 = std::move(fd1);

		EXPECT_TRUE(fd0.valid());
		EXPECT_EQ(fd0.value(), sysfd);

		EXPECT_TRUE(__fd_is_valid(fd0.value()));

		EXPECT_FALSE(fd1.valid());
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(ClosingDescriptor, Reset)
{
	int sysfd0 = __get_new_fd();
	int sysfd1 = __get_new_fd();
	int ret;

	{
		ClosingDescriptor fd = ClosingDescriptor(sysfd0);

		ret = fd.reset(sysfd1);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(ret, sysfd0);
		EXPECT_EQ(fd.value(), sysfd1);

		EXPECT_TRUE(__fd_is_valid(sysfd0));
		EXPECT_TRUE(__fd_is_valid(sysfd1));
	}

	EXPECT_TRUE(__fd_is_valid(sysfd0));
	EXPECT_FALSE(__fd_is_valid(sysfd1));

	::close(sysfd0);
}

TEST(ClosingDescriptor, Close)
{
	int sysfd = __get_new_fd();

	{
		ClosingDescriptor fd = ClosingDescriptor(sysfd);

		fd.close();

		EXPECT_FALSE(fd.valid());
		EXPECT_FALSE(__fd_is_valid(sysfd));
	}
}
