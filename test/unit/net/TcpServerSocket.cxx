#include <metasys/net/TcpServerSocket.hxx>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cassert>
#include <cerrno>

#include <gtest/gtest.h>

#include <metasys/net/InetAddress.hxx>
#include <metasys/sys/ErrnoException.hxx>


using metasys::ErrnoException;
using metasys::InetAddress;
using metasys::InterruptException;
using metasys::TcpServerSocket;


static inline bool __fd_is_valid(int fd)
{
	return (::fcntl(fd, F_GETFD) >= 0);
}

static inline uint16_t __find_free_tcp_port()
{
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sin;
	uint16_t port;
	int ret, val;

	assert(fd >= 0);

	port = 1024;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	val = 1;
	ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val));
	assert(ret == 0);

	do {
		assert(port < 65535);

		ret = ::bind(fd, (struct sockaddr *) &sin, sizeof (sin));

		if (ret < 0) {
			assert(errno == EADDRINUSE);
			port += 1;
			sin.sin_port = htons(port);
		}
	} while (ret < 0);

	::close(fd);
	return port;
}

static bool __try_connect_tcp_port(uint16_t port)
{
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sin;
	int ret;

	assert(fd >= 0);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	ret = ::connect(fd, (struct sockaddr *) &sin, sizeof (sin));

	::close(fd);

	return (ret == 0);
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


TEST(TcpServerSocket, Unassigned)
{
	TcpServerSocket sock;

	EXPECT_FALSE(sock.valid());
}

TEST(TcpServerSocket, ConstructFromSysBindListen)
{
	int sysfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	uint16_t port = __find_free_tcp_port();

	ASSERT_TRUE(sysfd >= 0);

	{
		TcpServerSocket sock = TcpServerSocket(sysfd);

		EXPECT_TRUE(sock.valid());
		EXPECT_EQ(sock.value(), sysfd);
		EXPECT_TRUE(__fd_is_valid(sock.value()));

		sock.bind(InetAddress::localhost(port));
		sock.listen();

		EXPECT_TRUE(__try_connect_tcp_port(port));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpServerSocket, OpenBindListen)
{
	uint16_t port = __find_free_tcp_port();
	int sysfd;

	{
		TcpServerSocket sock;

		EXPECT_FALSE(sock.valid());

		sock.open();

		EXPECT_TRUE(sock.valid());
		sysfd = sock.value();
		EXPECT_TRUE(__fd_is_valid(sysfd));

		sock.bind(InetAddress(port));
		sock.listen();

		EXPECT_TRUE(__try_connect_tcp_port(port));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpServerSocket, ListenInit)
{
	uint16_t port = __find_free_tcp_port();
	int sysfd;

	{
		TcpServerSocket sock = TcpServerSocket::listeninit
			(InetAddress(port));

		EXPECT_TRUE(sock.valid());
		sysfd = sock.value();
		EXPECT_TRUE(__fd_is_valid(sysfd));

		EXPECT_TRUE(__try_connect_tcp_port(port));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpServerSocket, ListenInitReusePort)
{
	uint16_t port = __find_free_tcp_port();
	int sysfd0, sysfd1;

	{
		TcpServerSocket sock0 = TcpServerSocket::listeninit
			(InetAddress(port));
		TcpServerSocket sock1 = TcpServerSocket::listeninit
			(InetAddress(port));

		EXPECT_TRUE(sock0.valid());
		sysfd0 = sock0.value();
		EXPECT_TRUE(__fd_is_valid(sysfd0));

		EXPECT_TRUE(sock1.valid());
		sysfd1 = sock1.value();
		EXPECT_TRUE(__fd_is_valid(sysfd1));

		EXPECT_TRUE(__try_connect_tcp_port(port));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd0));
	EXPECT_FALSE(__fd_is_valid(sysfd1));
}

TEST(TcpServerSocket, ListenInitNoReusePort)
{
	uint16_t port = __find_free_tcp_port();
	int sysfd;

	{
		TcpServerSocket sock = TcpServerSocket::listeninit<false>
			(InetAddress(port));

		EXPECT_TRUE(sock.valid());
		sysfd = sock.value();
		EXPECT_TRUE(__fd_is_valid(sysfd));

		EXPECT_THROW(TcpServerSocket::listeninit(InetAddress(port)),
			     ErrnoException<EADDRINUSE>);

		EXPECT_TRUE(__try_connect_tcp_port(port));
	}

	EXPECT_FALSE(__fd_is_valid(sysfd));
}

TEST(TcpServerSocket, AcceptInterrupt)
{
	uint16_t port = __find_free_tcp_port();

	{
		TcpServerSocket sock = TcpServerSocket::listeninit
			(InetAddress(port));

		EXPECT_TRUE(sock.valid());
		EXPECT_TRUE(__fd_is_valid(sock.value()));

		__send_nop_signal(1);

		EXPECT_THROW(sock.accept(), InterruptException);

		__wait_nop_signal();
	}
}
