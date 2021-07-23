#include <metasys/io/ReadableDescriptor.hxx>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include <metasys/sys/FileDescriptor.hxx>
#include <metasys/sys/ErrnoException.hxx>


using metasys::FileDescriptor;
using metasys::InterruptException;
using metasys::ReadableDescriptor;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}

static inline int __get_new_fd()
{
	return ::dup(STDIN_FILENO);
}

static inline void __get_pipe(int fds[2])
{
	int ret = ::pipe(fds);
	assert(ret == 0);
}

static inline int __replace_stdin(int fd)
{
	int cpy = ::dup(STDIN_FILENO);
	int ret;

	assert(cpy >= 0);

	ret = ::dup2(fd, STDIN_FILENO);

	assert(ret == STDIN_FILENO);

	return cpy;
}

static volatile bool handler_waiting = false;
static void nop_handler(int)
{
	handler_waiting = false;
}

static inline void __send_nop_signal(unsigned int seconds)
{
	struct sigaction sa;
	int ret;

	assert(handler_waiting == false);

	::sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = nop_handler;

	ret = ::sigaction(SIGALRM, &sa, nullptr);
	assert(ret == 0);

	::alarm(seconds);

	handler_waiting = true;
}

static inline void __wait_nop_signal()
{
	while (handler_waiting)
		asm volatile ("pause");
}


TEST(ReadableDescriptor, Unassigned)
{
	ReadableDescriptor fd;

	EXPECT_FALSE(fd.valid());
}

TEST(ReadableDescriptor, ConstructFromSys)
{
	int sysfd = __get_new_fd();

	{
		ReadableDescriptor fd = ReadableDescriptor(sysfd);

		EXPECT_TRUE(fd.valid());
		EXPECT_EQ(fd.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd.value()));
	}

	EXPECT_TRUE(__fd_is_valid(sysfd));

	::close(sysfd);
}

TEST(ReadableDescriptor, ConstructFromOther)
{
	int sysfd = __get_new_fd();

	{
		ReadableDescriptor fd0 = ReadableDescriptor(sysfd);
		ReadableDescriptor fd1 = std::move(fd0);

		EXPECT_FALSE(fd0.valid());

		EXPECT_TRUE(fd1.valid());
		EXPECT_EQ(fd1.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(fd1.value()));
	}

	EXPECT_TRUE(__fd_is_valid(sysfd));

	::close(sysfd);
}

TEST(ReadableDescriptor, Read)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		ReadableDescriptor fd = ReadableDescriptor(pfds[0]);
		char buf[5];
		size_t ret;

		EXPECT_TRUE(fd.valid());

		::write(pfds[1], "0123", 4);

		buf[4] = 12;

		ret = fd.read(buf, 4);

		EXPECT_EQ(ret, 4);
		EXPECT_EQ(buf[0], '0');
		EXPECT_EQ(buf[1], '1');
		EXPECT_EQ(buf[2], '2');
		EXPECT_EQ(buf[3], '3');
		EXPECT_EQ(buf[4], 12);
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(ReadableDescriptor, ReadEof)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		ReadableDescriptor fd = ReadableDescriptor(pfds[0]);
		char buf[3];
		size_t ret;

		EXPECT_TRUE(fd.valid());

		::write(pfds[1], "01", 2);
		::close(pfds[1]);

		buf[2] = 12;

		ret = fd.read(buf, 3);

		EXPECT_EQ(ret, 2);
		EXPECT_EQ(buf[0], '0');
		EXPECT_EQ(buf[1], '1');
		EXPECT_EQ(buf[2], 12);

		buf[0] = 13;
		buf[1] = 14;

		ret = fd.read(buf, 3);

		EXPECT_EQ(ret, 0);
		EXPECT_EQ(buf[0], 13);
		EXPECT_EQ(buf[1], 14);
		EXPECT_EQ(buf[2], 12);
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));

	::close(pfds[0]);
}

TEST(ReadableDescriptor, ReadInterrupted)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		ReadableDescriptor fd = ReadableDescriptor(pfds[0]);
		char buf[5];

		EXPECT_TRUE(fd.valid());

		__send_nop_signal(1);

		EXPECT_THROW(fd.read(buf, 5), InterruptException);

		__wait_nop_signal();
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(ReadableDescriptor, ReadHalfInterrupted)
{
	int pfds[2];

	__get_pipe(pfds);

	{
		ReadableDescriptor fd = ReadableDescriptor(pfds[0]);
		char buf[5];
		size_t ret;

		EXPECT_TRUE(fd.valid());

		::write(pfds[1], "01", 2);

		__send_nop_signal(1);

		ret = fd.read(buf, 5);

		__wait_nop_signal();

		EXPECT_EQ(ret, 2);
		EXPECT_EQ(buf[0], '0');
		EXPECT_EQ(buf[1], '1');
	}

	EXPECT_TRUE(__fd_is_valid(pfds[0]));
	EXPECT_TRUE(__fd_is_valid(pfds[1]));

	::close(pfds[0]);
	::close(pfds[1]);
}

TEST(ReadableDescriptor, Stdin)
{
	EXPECT_TRUE(metasys::stdin().valid());
	EXPECT_EQ(metasys::stdin().value(), STDIN_FILENO);
}

TEST(ReadableDescriptor, StdinRead)
{
	int pfds[2];
	int oldstdin;
	size_t ret;
	char c;

	__get_pipe(pfds);
	oldstdin = __replace_stdin(pfds[0]);

	::write(pfds[1], "A", 1);

	ret = metasys::stdin().read(&c, 1);

	EXPECT_EQ(ret, 1);
	EXPECT_EQ(c, 'A');

	__replace_stdin(oldstdin);

	::close(pfds[0]);
	::close(pfds[1]);
}
