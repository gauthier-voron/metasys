#ifndef _INCLUDE_METASYS_FS_CREATABLE_HXX_
#define _INCLUDE_METASYS_FS_CREATABLE_HXX_


#include <concepts>
#include <string>
#include <utility>


namespace metasys {


namespace details {


template<typename Base>
class Creatable : public Base
{
 public:
	using Base::Base;


	Creatable(const Base &other)
		: Base(other)
	{
	}

	Creatable(Base &&other)
		: Base(std::move(other))
	{
	}


	using Base::create;

	template<typename ErrHandler>
	auto create(const std::string &pathname, int flags, mode_t mode,
		    ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		return create(pathname.c_str(), flags, mode,
			      std::forward<ErrHandler>(handler));
	}

	template<typename Pathname, typename ErrHandler>
	auto create(Pathname &&pathname, mode_t mode, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return create(std::forward<Pathname>(pathname), 0, mode,
			      std::forward<ErrHandler>(handler));
	}

	template<typename Pathname>
	void create(Pathname &&pathname, int flags, mode_t mode)
	{
		create(std::forward<Pathname>(pathname), flags, mode,
		       [](int ret) {
			if (ret < 0) [[unlikely]]
				Base::throwcreate();
		});
	}

	template<typename Pathname>
	void create(Pathname &&pathname, mode_t mode)
	{
		create(std::forward<Pathname>(pathname), 0, mode);
	}


	using Base::createinit;

	template<typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static Base createinit(const std::string &pathname, int flags,
			       mode_t mode, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return createinit(pathname.c_str(), flags, mode,
				  std::forward<ErrHandler>(handler));
	}

	template<typename Pathname, typename ErrHandler>
	requires std::invocable<ErrHandler, int>
	static Base createinit(Pathname &&pathname, mode_t mode,
			       ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return createinit(std::forward<Pathname>(pathname), 0, mode,
				  std::forward<ErrHandler>(handler));
	}

	template<typename Pathname>
	static Base createinit(Pathname &&pathname, int flags, mode_t mode)
	{
		return createinit(std::forward<Pathname>(pathname), flags,
				  mode, [](int ret) {
			if (ret < 0) [[unlikely]]
				Base::throwcreate();
		});
	}

	template<typename Pathname>
	static Base createinit(Pathname &&pathname, mode_t mode)
	{
		return createinit(std::forward<Pathname>(pathname), 0, mode);
	}
};


}


}


#endif
