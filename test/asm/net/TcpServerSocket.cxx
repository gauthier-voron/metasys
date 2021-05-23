#include <metasys/net/TcpServerSocket.hxx>

#include <netinet/in.h>
#include <sys/socket.h>

#include <asmcmp.hxx>

#include <metasys/net/InetAddress.hxx>
#include <metasys/net/TcpSocket.hxx>


using metasys::InetAddress;
using metasys::TcpServerSocket;
using metasys::TcpSocket;


Model(ConnectBindListenUnsafe)
{
	int fd;

	fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	Overhead(struct sockaddr_in sin, struct sockaddr_in sin = {});
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9000);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	::bind(fd, (struct sockaddr *) &sin, sizeof (sin));

	::listen(fd, 10);

	Overhead(, if (fd >= 0))
	::close(fd);
}
Test(ConnectBindListenUnsafe)
{
	TcpServerSocket fd;

	fd.open([](auto){});
	fd.bind(InetAddress::localhost(9000), [](auto){});
	fd.listen(10, [](auto){});
}

Model(ListenInitSafe)
{
	Overhead(struct sockaddr_in sin, struct sockaddr_in sin = {});
	int fd;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(9000);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
		return;

	if (::bind(fd, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		goto out;

	if (::listen(fd, 10) < 0)
		goto out;

	asm volatile ("nop");

	::close(fd);
	return;
 out:
	::close(fd);
	throw 0;
}
Test(ListenInitSafe)
{
	TcpServerSocket fd = TcpServerSocket::listeninit<false>
		(InetAddress::localhost(9000), 10);

	asm volatile ("nop");
}

Model(ListenInitAcceptSafe)
{
	Overhead(struct sockaddr_in sin, struct sockaddr_in sin = {});
	socklen_t slen;
	int fd, conn;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(9000);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
		return;

	if (::bind(fd, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		goto out;

	if (::listen(fd, 10) < 0)
		goto out;

	slen = sizeof (sin);
	conn = ::accept4(fd, (struct sockaddr *) &sin, &slen, 0);
	if (conn < 0)
		goto out;

	asm volatile ("nop");

	::close(conn);
	::close(fd);
	return;
 out:
	::close(fd);
	throw 0;
}
Test(ListenInitAcceptSafe)
{
	InetAddress addr = InetAddress::localhost(9000);
	TcpServerSocket fd = TcpServerSocket::listeninit<false>(addr, 10);
	TcpSocket conn = fd.accept(&addr);

	asm volatile ("nop");
}
