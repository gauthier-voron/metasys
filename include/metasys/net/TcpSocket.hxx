#ifndef _INCLUDE_METASYS_NET_TCPSOCKET_HXX_
#define _INCLUDE_METASYS_NET_TCPSOCKET_HXX_


#include <cassert>
#include <cerrno>
#include <concepts>

#include <metasys/io/ReadableDescriptor.hxx>
#include <metasys/io/WritableDescriptor.hxx>
#include <metasys/net/InetAddress.hxx>
#include <metasys/net/TcpSocketDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


namespace details {


using TcpSocketBase =
	WritableInterface
	<ReadableInterface
	 <TcpSocketDescriptor>>;


}


class TcpSocket : public details::TcpSocketBase
{
 public:
	using details::TcpSocketBase::TcpSocketBase;


	template<typename ErrHandler>
	auto connect(const struct sockaddr_in *addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		const struct sockaddr *saddr;
		int ret;

		assert(valid());

		saddr = reinterpret_cast<const struct sockaddr *> (addr);
		ret = ::connect(value(), saddr, sizeof (*addr));

		return handler(ret);
	}

	template<typename ErrHandler>
	auto connect(const InetAddress &addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return connect(addr.saddrin(),
			       std::forward<ErrHandler>(handler));
	}

	void connect(const struct sockaddr_in *addr)
	{
		connect(addr, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwconnect();
		});
	}

	void connect(const InetAddress &addr)
	{
		connect(addr.saddrin());
	}

	[[noreturn]] static void throwconnect()
	{
		assert(errno != EAFNOSUPPORT);
		assert(errno != EALREADY);
		assert(errno != EBADF);
		assert(errno != EFAULT);
		assert(errno != EISCONN);
		assert(errno != ENOTSOCK);
		assert(errno != EPROTOTYPE);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto disconnect(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		struct sockaddr saddr;
		int ret;

		assert(valid());

		saddr.sa_family = AF_UNSPEC;
		ret = ::connect(value(), &saddr, sizeof (saddr));

		return handler(ret);
	}

	void disconnect()
	{
		disconnect([](int ret) {
			if (ret < 0) [[unlikely]]
				throwdisconnect();
		});
	}

	[[noreturn]] static void throwdisconnect()
	{
		throwconnect();
	}


	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpSocket connectinit(const struct sockaddr_in *addr, int flags,
				     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int err;
		TcpSocket ret = TcpSocket::openinit(flags, [&err](int r){
			err = r;
		});

		if (err < 0) [[unlikely]]
			goto err;

		if ((err = ret.connect(addr, [](int r) { return r; })) < 0)
			goto err_close;

		return ret;
	 err_close:
		ret.close();
	 err:
		handler(err);
		return ret;
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpSocket connectinit(const struct sockaddr_in *addr,
				     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return connectinit(addr, 0, std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpSocket connectinit(const InetAddress &addr, int flags,
				     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return connectinit(addr.saddrin(), flags,
				   std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpSocket connectinit(const InetAddress &addr,
				     ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return connectinit(addr.saddrin(),
				   std::forward<ErrHandler>(handler));
	}

	static TcpSocket connectinit(const struct sockaddr_in *addr,
				     int flags = 0)
	{
		TcpSocket ret = TcpSocket::openinit(flags);

		ret.connect(addr);

		return ret;
	}

	static TcpSocket connectinit(const InetAddress &addr, int flags = 0)
	{
		return connectinit(addr.saddrin(), flags);
	}
};


}


#endif
