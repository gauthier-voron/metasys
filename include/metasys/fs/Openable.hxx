#ifndef _INCLUDE_METASYS_FS_OPENABLE_HXX_
#define _INCLUDE_METASYS_FS_OPENABLE_HXX_


#include <concepts>
#include <string>
#include <utility>


namespace metasys {


namespace details {


template<typename Base>
class Openable : public Base
{
 public:
	using Base::Base;


	Openable(const Base &other)
		: Base(other)
	{
	}

	Openable(Base &&other)
		: Base(std::move(other))
	{
	}


	using Base::open;

	template<typename ErrHandler>
	auto open(const std::string &pathname, int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return open(pathname.c_str(), flags,
			    std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto open(const char *pathname, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return open(pathname, 0, std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	auto open(const std::string &pathname, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return open(pathname.c_str(),
			    std::forward<ErrHandler>(handler));
	}

	template<typename Pathname>
	void open(Pathname &&pathname, int flags = 0)
	{
		open(std::forward<Pathname>(pathname), flags, [](int ret) {
			if (ret < 0) [[unlikely]]
				Base::throwopen();
		});
	}


	using Base::openinit;

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static Openable openinit(const std::string &pathname, int flags,
				 ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return openinit(pathname.c_str(), flags,
				std::forward<ErrHandler>(handler));
	}

	template<typename Pathname, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static Openable openinit(Pathname &&pathname, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return openinit(std::forward<Pathname>(pathname), 0,
				std::forward<ErrHandler>(handler));
	}

	template<typename Pathname>
	static Openable openinit(Pathname &&pathname, int flags = 0)
	{
		return openinit(std::forward<Pathname>(pathname), flags,
				[](int ret) {
			if (ret < 0) [[unlikely]]
				Base::throwopen();
		});
	}
};


}


}


#endif
