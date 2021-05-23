#ifndef _INCLUDE_METASYS_NET_TCPSERVERSOCKET_HXX_
#define _INCLUDE_METASYS_NET_TCPSERVERSOCKET_HXX_


#include <cassert>
#include <concepts>

#include <metasys/net/TcpSocket.hxx>
#include <metasys/net/TcpSocketDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


class TcpServerSocket : public details::TcpSocketDescriptor
{
 public:
	using details::TcpSocketDescriptor::TcpSocketDescriptor;


	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	TcpSocket accept(struct sockaddr_in *from, int flags,
			 ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		socklen_t slen = sizeof (*from);
		int ret;

		assert(valid());

		ret = ::accept4(value(),
				reinterpret_cast<struct sockaddr *> (from),
				&slen, flags);

		assert(slen == sizeof (*from));

		handler(ret);

		return TcpSocket(ret);
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	TcpSocket accept(InetAddress *from, int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return accept(from->saddrin(), flags,
			      std::forward<ErrHandler>(handler));
	}

	template<typename Address, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
		&& (!std::convertible_to<Address, int>)
	TcpSocket accept(Address &&from, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return accept(std::forward<Address>(from), 0,
			      std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	TcpSocket accept(int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return accept(static_cast<struct sockaddr_in *> (nullptr),
			      flags, std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	TcpSocket accept(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return accept(static_cast<struct sockaddr_in *> (nullptr),
			      0, std::forward<ErrHandler>(handler));
	}

	TcpSocket accept(struct sockaddr_in *from = nullptr, int flags = 0)
	{
		return accept(from, flags, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwaccept();
		});
	}

	TcpSocket accept(InetAddress *from, int flags = 0)
	{
		return accept(from->saddrin(), flags);
	}

	TcpSocket accept(int flags)
	{
		return accept(static_cast<struct sockaddr_in *> (nullptr),
			      flags);
	}

	static void throwaccept()
	{
		assert(errno != EBADF);
		assert(errno != EFAULT);
		assert(errno != EINVAL);
		assert(errno != ENOTSOCK);
		assert(errno != EOPNOTSUPP);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto listen(int backlog, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::listen(value(), backlog));
	}

	void listen(int backlog = 32)
	{
		listen(backlog, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwlisten();
		});
	}

	template<bool ReusePort = true, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpServerSocket listeninit(const struct sockaddr_in *addr,
					  int backlog, int flags,
					  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int err;
		TcpServerSocket ret = TcpServerSocket::openinit(flags,
								[&err](int r) {
			err = r;
		});

		if (err < 0) [[unlikely]]
			goto err;

		if constexpr (ReusePort) {
			err = ret.setreuseport(true, [](int r) { return r; });
			if (err < 0) [[unlikely]]
				goto err_close;
		}

		err = ret.bind(addr, [](int r) { return r; });
		if (err < 0) [[unlikely]]
			goto err_close;

		err = ret.listen(backlog, [](int r) { return r; });
		if (err < 0) [[unlikely]]
			goto err_close;

		return ret;
	 err_close:
		ret.close();
	 err:
		handler(err);
		return ret;
	}

	template<bool ReusePort = true, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpServerSocket listeninit(const InetAddress &addr, int backlog,
					  int flags, ErrHandler &&handler)
	{
		return listeninit<ReusePort>
			(addr.saddrin(), backlog, flags,
			 std::forward<ErrHandler>(handler));
	}

	template<bool ReusePort = true, typename Address, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpServerSocket listeninit(Address &&addr, int backlog,
					  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return listeninit(std::forward<Address>(addr), backlog, 0,
				  std::forward<ErrHandler>(handler));
	}

	template<bool ReusePort = true, typename Address, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static TcpServerSocket listeninit(Address &&addr, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return listeninit(std::forward<Address>(addr), 32, 0,
				  std::forward<ErrHandler>(handler));
	}

	template<bool ReusePort = true>
	static TcpServerSocket listeninit(const struct sockaddr_in *addr,
					  int backlog = 32, int flags = 0)
	{
		TcpServerSocket ret = TcpServerSocket::openinit(flags);

		if constexpr (ReusePort)
			ret.setreuseport(true);

		ret.bind(addr);

		ret.listen(backlog);

		return ret;
	}

	template<bool ReusePort = true>
	static TcpServerSocket listeninit(const InetAddress &addr,
					  int backlog = 32, int flags = 0)
	{
		return listeninit<ReusePort>(addr.saddrin(), backlog, flags);
	}

	static void throwlisten()
	{
		assert(errno != EBADF);
		assert(errno != ENOTSOCK);
		assert(errno != EOPNOTSUPP);

		SystemException::throwErrno();
	}
};


}


#endif
