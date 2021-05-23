#ifndef _INCLUDE_METASYS_META_STATICOPTIONAL_HXX_
#define _INCLUDE_METASYS_META_STATICOPTIONAL_HXX_


#include <concepts>
#include <functional>
#include <utility>


namespace metasys {


namespace details {


template<bool Enable, typename T>
class StaticOptional;


template<typename T>
class StaticOptional<true, T>
{
	T  _value;


 public:
	template<typename ... Args>
	requires std::constructible_from<T, Args ...>
	StaticOptional(Args && ... args)
		: _value(std::forward<Args>(args) ...)
	{
	}

	template<typename Function, typename ... Args>
	requires std::invocable<Function, Args ...>
	StaticOptional(Function &&func, Args && ... args)
		: _value(std::invoke(std::forward<Function>(func),
				     std::forward<Args>(args) ...))
	{
	}

	static constexpr bool enabled() noexcept
	{
		return true;
	}

	template<typename Function, typename ... Args>
	requires std::invocable<Function, T, Args ...>
	void invoke(Function &&func, Args && ... args)
	{
		std::invoke(std::forward<Function>(func), _value,
			    std::forward<Args>(args) ...);
	}
};

template<typename T>
class StaticOptional<false, T>
{
 public:
	template<typename ... Args>
	requires std::constructible_from<T, Args ...>
	StaticOptional(Args && ...)
	{
	}

	template<typename Function, typename ... Args>
	requires std::invocable<Function, Args ...>
	StaticOptional(Function &&, Args && ...)
	{
	}

	static constexpr bool enabled() noexcept
	{
		return false;
	}

	template<typename Function, typename ... Args>
	requires std::invocable<Function, T, Args ...>
	void invoke(Function &&, Args && ...)
	{
	}
};


}


}


#endif
