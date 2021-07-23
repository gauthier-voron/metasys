#ifndef _INCLUDE_METASYS_NET_SOCKETDESCRIPTOR_HXX_
#define _INCLUDE_METASYS_NET_SOCKETDESCRIPTOR_HXX_


#include <sys/socket.h>

#include <cassert>
#include <cerrno>
#include <concepts>

#include <metasys/sys/ClosingDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


template<int Domain, int Type, int Protocol>
class SocketDescriptor : public ClosingDescriptor
{
 public:
	using ClosingDescriptor::ClosingDescriptor;


	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto open(int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);

		reset(::socket(Domain, Type | flags, Protocol));

		return handler(value());
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto open(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		return open(0, std::forward<ErrHandler>(handler));
	}

	void open(int flags = 0)
	{
		open(flags, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwopen();
		});
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static SocketDescriptor openinit(int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int fd = ::socket(Domain, Type | flags, Protocol);

		handler(fd);

		return SocketDescriptor(fd);
	}

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static SocketDescriptor openinit(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return openinit(0, std::forward<ErrHandler>(handler));
	}

	static SocketDescriptor openinit(int flags = 0)
	{
		return openinit(flags, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwopen();
		});
	}

	static void throwopen()
	{
		assert(errno != EINVAL);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto getsockopt(int level, int optname, void *optval,
			socklen_t *optlen, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::getsockopt(value(), level, optname, optval,
					    optlen));
	}

	template<typename Value, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto getsockopt(int level, int optname, Value *optval,
			ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		socklen_t slen = sizeof (Value);

		return getsockopt(level, optname, optval, &slen,
				  std::forward<ErrHandler>(handler));
	}

	void getsockopt(int level, int optname, void *optval,
			socklen_t *optlen) noexcept
	{
		getsockopt(level, optname, optval, optlen, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwgetsockopt();
		});
	}

	template<typename Value>
	void getsockopt(int level, int optname, Value *optval) noexcept
	{
		socklen_t slen = sizeof (Value);

		getsockopt(level, optname, optval, &slen);
	}

	static void throwgetsockopt() noexcept
	{
		assert(errno != EBADF);
		assert(errno != EFAULT);
		assert(errno != EINVAL);
		assert(errno != ENOPROTOOPT);
		assert(errno != ENOTSOCK);
	}


	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto setsockopt(int level, int optname, const void *optval,
			socklen_t optlen, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::setsockopt(value(), level, optname,
					    optval, optlen));
	}

	template<typename Value, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	auto setsockopt(int level, int optname, const Value &optval,
			ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return setsockopt(level, optname, &optval, sizeof (optval),
				  std::forward<ErrHandler>(handler));
	}

	void setsockopt(int level, int optname, const void *optval,
			socklen_t optlen)
	{
		setsockopt(level, optname, optval, optlen, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwsetsockopt();
		});
	}

	template<typename Value>
	void setsockopt(int level, int optname, const void *optval,
			socklen_t optlen)
	{
		setsockopt(level, optname, &optval, sizeof (optval));
	}

	static void throwsetsockopt()
	{
		assert(errno != EBADF);
		assert(errno != EDOM);
		assert(errno != EINVAL);
		assert(errno != EISCONN);
		assert(errno != ENOPROTOOPT);
		assert(errno != ENOTSOCK);

		SystemException::throwErrno();
	}


	int geterror() noexcept
	{
		int ret;

		getsockopt(SOL_SOCKET, SO_ERROR, &ret);

		return ret;
	}
};


}


#endif
