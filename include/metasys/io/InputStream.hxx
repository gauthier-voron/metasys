#ifndef _INCLUDE_METASYS_IO_INPUTSTREAM_HXX_
#define _INCLUDE_METASYS_IO_INPUTSTREAM_HXX_


#include <cstdint>
#include <cstdlib>

#include <concepts>


namespace metasys {


template<typename T>
concept UnitInputStream = requires (T a)
{
	{ a.read() } -> std::same_as<int16_t>;
};

template<typename T>
concept BatchInputStream = requires (T a, void *buf, size_t len)
{
	{ a.read(buf, len) } -> std::same_as<size_t>;
};


template<typename T>
concept InputStream = UnitInputStream<T> || BatchInputStream<T>;


}


#endif
