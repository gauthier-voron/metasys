#include <metasys/sched/EpollDescriptor.hxx>

#include <fcntl.h>
#include <sys/epoll.h>

#include <gtest/gtest.h>

#include <array>

#include <metasys/sys/FileDescriptor.hxx>


using metasys::EpollDescriptor;
using metasys::EpollEvent;
using metasys::FileDescriptor;
using std::array;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}

static inline void __get_pipe(int fds[2])
{
	ASSERT_EQ(::pipe(fds), 0);
}


TEST(EpollDescriptor, Unassigned)
{
	EpollDescriptor fd;

	EXPECT_FALSE(fd.valid());
}

TEST(EpollDescriptor, ConstructFromSys)
{
	int sysfd = ::epoll_create(1);

	{
		EpollDescriptor fd = EpollDescriptor(sysfd);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(fd.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(EpollDescriptor, Create)
{
	int sysfd;

	{
		EpollDescriptor fd;

		fd.create();

		EXPECT_TRUE(fd.valid());

		sysfd = fd.value();

		EXPECT_TRUE(__fd_is_valid(sysfd));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(EpollDescriptor, Createinit)
{
	int sysfd;

	{
		EpollDescriptor fd = EpollDescriptor::createinit();

		EXPECT_TRUE(fd.valid());

		sysfd = fd.value();

		EXPECT_TRUE(__fd_is_valid(sysfd));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(EpollDescriptor, AddFd)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		EpollDescriptor fd = EpollDescriptor::createinit();
		EpollEvent ev = EpollEvent(EPOLLIN, 0ul);

		EXPECT_TRUE(fd.valid());

		fd.add(pfds[0], ev);
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(EpollDescriptor, AddDescriptor)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		EpollDescriptor fd = EpollDescriptor::createinit();
		EpollEvent ev = EpollEvent(EPOLLIN, 0ul);
		FileDescriptor d = FileDescriptor(pfds[0]);

		EXPECT_TRUE(fd.valid());

		fd.add(d, ev);
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(EpollDescriptor, Wait)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		EpollDescriptor fd = EpollDescriptor::createinit();
		EpollEvent evi = EpollEvent(EPOLLIN, 1ul);
		EpollEvent evo = EpollEvent(EPOLLIN, 0ul);

		EXPECT_TRUE(fd.valid());

		fd.add(pfds[0], evi);

		ASSERT_EQ(::write(pfds[1], "\0", 1), 1);

		EXPECT_EQ(fd.wait(&evo, 1, 0), 1);
		EXPECT_EQ(evo.data(), 1ul);
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(EpollDescriptor, WaitTimeout)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		EpollDescriptor fd = EpollDescriptor::createinit();
		EpollEvent evi = EpollEvent(EPOLLIN, 1ul);
		EpollEvent evo = EpollEvent(EPOLLIN, 0ul);

		EXPECT_TRUE(fd.valid());

		fd.add(pfds[0], evi);

		EXPECT_EQ(fd.wait(&evo, 1, 0), 0);
		EXPECT_EQ(evo.data(), 0ul);
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(EpollDescriptor, WaitMany)
{
	int pfds_a[2];
	int pfds_b[2];

	__get_pipe(pfds_a);
	__get_pipe(pfds_b);

	{
		EpollDescriptor fd = EpollDescriptor::createinit();
		array<EpollEvent<uint64_t>, 4> arr;

		EXPECT_TRUE(fd.valid());

		fd.add(pfds_a[0], EpollEvent(EPOLLIN, 10ul));
		fd.add(pfds_b[0], EpollEvent(EPOLLIN, 20ul));

		ASSERT_EQ(::write(pfds_a[1], "\0", 1), 1);
		ASSERT_EQ(::write(pfds_b[1], "\0", 1), 1);

		EXPECT_EQ(fd.wait(&arr, 0), 2);

		EXPECT_TRUE(arr[0].data() == 10ul || arr[1].data() == 10ul);
		EXPECT_TRUE(arr[0].data() == 20ul || arr[1].data() == 20ul);
		EXPECT_TRUE(arr[0].data() != arr[1].data());
	}

	EXPECT_TRUE(__fd_is_valid(pfds_a[0]));
	EXPECT_TRUE(__fd_is_valid(pfds_a[1]));
	EXPECT_TRUE(__fd_is_valid(pfds_b[0]));
	EXPECT_TRUE(__fd_is_valid(pfds_b[1]));

	::close(pfds_a[0]);
	::close(pfds_a[1]);
	::close(pfds_b[0]);
	::close(pfds_b[1]);
}
