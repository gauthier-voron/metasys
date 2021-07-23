#include <metasys/net/TcpSocket.hxx>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <cassert>

#include <gtest/gtest.h>


using metasys::TcpSocket;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}

static inline int __open_fd()
{
	int ret = ::socket(AF_INET, SOCK_STREAM, 0);

	assert(ret >= 0);

	return ret;
}


TEST(TcpSocket, Unassigned)
{
	TcpSocket sock;

	EXPECT_FALSE(sock.valid());
}

TEST(TcpSocket, ConstructFromSys)
{
	int sysfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ASSERT_TRUE(sysfd >= 0);

	{
		TcpSocket sock = TcpSocket(sysfd);

		EXPECT_TRUE(sock.valid());
		EXPECT_EQ(sock.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(sock.value()));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpSocket, OpenInit)
{
	int sysfd;

	{
		TcpSocket sock = TcpSocket::openinit();

		EXPECT_TRUE(sock.valid());
		sysfd = sock.value();
		EXPECT_TRUE(__fd_is_valid(sysfd));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpSocket, Open)
{
	int sysfd;

	{
		TcpSocket sock;

		EXPECT_FALSE(sock.valid());

		sock.open();

		EXPECT_TRUE(sock.valid());
		sysfd = sock.value();
		EXPECT_TRUE(__fd_is_valid(sysfd));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpSocket, ConnectInitFail)
{
	struct sockaddr_in sin;
	int sysfd0, sysfd1;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(1);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	sysfd0 = __open_fd();
	::close(sysfd0);

	TcpSocket sock = TcpSocket::connectinit(&sin, [](auto){});

	sysfd1 = __open_fd();
	::close(sysfd1);

	EXPECT_FALSE(sock.valid());
	EXPECT_EQ(sysfd0, sysfd1);
}

TEST(TcpSocket, GetNoError)
{
	TcpSocket sock = TcpSocket::openinit();
	int err = sock.geterror();

	EXPECT_EQ(err, 0);
}

TEST(TcpSocket, GetError)
{
	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(1);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	TcpSocket sock = TcpSocket::openinit(SOCK_NONBLOCK);
	int err, ret = sock.connect(&sin, [](int ret) {
		return (ret < 0) ? errno : ret;
	});
	fd_set fds;

	EXPECT_EQ(ret, EINPROGRESS);

	FD_ZERO(&fds);
	FD_SET(sock.value(), &fds);

	::select(sock.value() + 1, nullptr, &fds, nullptr, nullptr);

	err = sock.geterror();

	EXPECT_EQ(err, ECONNREFUSED);
}
