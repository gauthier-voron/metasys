#include <metasys/net/TcpSocket.hxx>

#include <netinet/in.h>
#include <sys/socket.h>

#include <asmcmp.hxx>

#include <metasys/net/InetAddress.hxx>


using metasys::InetAddress;
using metasys::TcpSocket;


Model(ConnectInitUnsafe)
{
	// TODO: change `InetAddress` so it does not do the `sin = {}`.
	//
	// The `struct sockaddr_in` contains a field `sin_zero` to pad it at
	// the size of `struct sockaddr`.
	// Because `InetAddress` constructors are `constexpr`, g++ needs to
	// initialize even this `sin_zero` field whereas nobody needs it.
	//
	Overhead(struct sockaddr_in sin, struct sockaddr_in sin = {});
	int fd;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(9000);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	::connect(fd, (struct sockaddr *) &sin, sizeof (sin));

	::close(fd);
}
Test(ConnectInitUnsafe)
{
	InetAddress dest = InetAddress::localhost(9000);
	TcpSocket sock = TcpSocket::openinit([](auto){});

	sock.connect(dest, [](auto){});

	sock.close([](auto){});
}

Model(ConnectInitSafe)
{
	Overhead(struct sockaddr_in sin, struct sockaddr_in sin = {});
	int fd;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(9000);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) [[unlikely]]
		return;

	if (::connect(fd,(struct sockaddr*)&sin,sizeof (sin)) < 0) [[unlikely]]
		return;

	::close(fd);
}
Test(ConnectInitSafe)
{
	TcpSocket sock = TcpSocket::connectinit(InetAddress::localhost(9000));
}
