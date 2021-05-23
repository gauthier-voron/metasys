#ifndef _INCLUDE_METASYS_IO_OUTPUTSTREAM_HXX_
#define _INCLUDE_METASYS_IO_OUTPUTSTREAM_HXX_


#include <cstdint>
#include <cstdlib>

#include <concepts>


namespace metasys {


template<typename T>
concept UnitOutputStream = requires (T a, int8_t c)
{
	a.write(c);
};

template<typename T>
concept BatchOutputStream = requires (T a, const void *buf, size_t len)
{
	{ a.write(buf, len) } -> std::same_as<size_t>;
};


template<typename T>
concept OutputStream = UnitOutputStream<T> || BatchOutputStream<T>;


}


#endif
