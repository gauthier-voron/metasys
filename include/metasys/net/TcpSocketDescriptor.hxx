#ifndef _INCLUDE_METASYS_NET_TCPSOCKETDESCRIPTOR_HXX_
#define _INCLUDE_METASYS_NET_TCPSOCKETDESCRIPTOR_HXX_


#include <netinet/in.h>
#include <sys/socket.h>

#include <metasys/net/InetAddress.hxx>
#include <metasys/net/SocketDescriptor.hxx>


namespace metasys {


namespace details {


using TcpSocketDescriptorBase = metasys::SocketDescriptor<
	AF_INET, SOCK_STREAM, IPPROTO_TCP>;


class TcpSocketDescriptor : public TcpSocketDescriptorBase
{
 public:
	using TcpSocketDescriptorBase::TcpSocketDescriptorBase;

	TcpSocketDescriptor(TcpSocketDescriptorBase &&other) noexcept
		: TcpSocketDescriptorBase(std::move(other))
	{
	}


	template<typename ErrHandler>
	auto bind(const struct sockaddr_in *addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		const sockaddr *saddr;

		assert(valid());

		saddr = reinterpret_cast<const struct sockaddr *> (addr);
		return handler(::bind(value(), saddr, sizeof (*addr)));
	}

	template<typename ErrHandler>
	auto bind(const InetAddress &addr, ErrHandler &&handler)
	{
		return bind(addr.saddrin(), std::forward<ErrHandler>(handler));
	}

	void bind(const struct sockaddr_in *addr)
	{
		bind(addr, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwbind();
		});
	}

	void bind(const InetAddress &addr)
	{
		bind(addr.saddrin());
	}

	static void throwbind()
	{
		assert(errno != EBADF);
		assert(errno != EINVAL);
		assert(errno != ENOTSOCK);
		assert(errno != EFAULT);
		assert(errno != ENAMETOOLONG);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto setreuseaddr(bool allow, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return setsockopt(SOL_SOCKET, SO_REUSEADDR,
				  static_cast<int> (allow),
				  std::forward<ErrHandler>(handler));
	}

	void setreuseaddr(bool allow)
	{
		setreuseaddr(allow, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwsetsockopt();
		});
	}


	template<typename ErrHandler>
	auto setreuseport(bool allow, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return setsockopt(SOL_SOCKET, SO_REUSEPORT,
				  static_cast<int> (allow),
				  std::forward<ErrHandler>(handler));
	}

	void setreuseport(bool allow)
	{
		setreuseport(allow, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwsetsockopt();
		});
	}
};


}


}


#endif
