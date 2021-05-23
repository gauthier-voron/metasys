#ifndef _INCLUDE_METASYS_META_AUTOHOLDER_HXX_
#define _INCLUDE_METASYS_META_AUTOHOLDER_HXX_


#include <functional>
#include <utility>


namespace metasys {


namespace details {


template<typename T>
class AutoHolder
{
	T  _value;


 public:
	using type = T;

	template<typename Function, typename ... Args>
	AutoHolder(Function &&func, Args && ... args)
		: _value(std::invoke(std::forward<Function>(func),
				     std::forward<Args>(args) ...))
	{
	}

	static constexpr bool isvoid() noexcept
	{
		return false;
	}

	T &value() noexcept
	{
		return _value;
	}

	const T &value() const noexcept
	{
		return _value;
	}
};

template<>
class AutoHolder<void>
{
 public:
	using type = void;

	template<typename Function, typename ... Args>
	AutoHolder(Function &&func, Args && ... args)
	{
		std::invoke(std::forward<Function>(func),
			    std::forward<Args>(args) ...);
	}

	static constexpr bool isvoid() noexcept
	{
		return true;
	}
};

template<typename Function, typename ... Args>
AutoHolder(Function &&func, Args && ... args)
	-> AutoHolder<decltype (std::invoke(std::forward<Function>(func),
					    std::forward<Args>(args) ...))>;


}


}


#endif
