// Copyright 2021 Gauthier Voron
//
// This file is part of Metasys.
//
// Metasys is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Metasys is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// Metasys. If not, see <https://www.gnu.org/licenses/>.
//


#ifndef _INCLUDE_METASYS_NET_BITS_HXX_
#define _INCLUDE_METASYS_NET_BITS_HXX_


#include <cstdint>

#include <bit>


namespace metasys {


constexpr uint16_t bswap(uint16_t value) noexcept
{
	return (  ((value & 0x00ff) << 8)
		| ((value & 0xff00) >> 8));
}

constexpr uint32_t bswap(uint32_t value) noexcept
{
	return (  ((value & 0x000000ff) << 24)
	        | ((value & 0x0000ff00) <<  8)
		| ((value & 0x00ff0000) >>  8)
		| ((value & 0xff000000) >> 24));
}

constexpr uint64_t bswap(uint64_t value) noexcept
{
	return (  ((value & 0x00000000000000ff) << 56)
	        | ((value & 0x000000000000ff00) << 40)
	        | ((value & 0x0000000000ff0000) << 24)
	        | ((value & 0x00000000ff000000) <<  8)
		| ((value & 0x000000ff00000000) >>  8)
		| ((value & 0x0000ff0000000000) >> 24)
		| ((value & 0x00ff000000000000) >> 40)
		| ((value & 0xff00000000000000) >> 56));
}


constexpr uint8_t hton(uint8_t value) noexcept
{
	return value;
}

constexpr uint16_t hton(uint16_t value) noexcept
{
	if constexpr (std::endian::native == std::endian::little)
		return bswap(value);
	else
		return value;
}

constexpr uint32_t hton(uint32_t value) noexcept
{
	if constexpr (std::endian::native == std::endian::little)
		return bswap(value);
	else
		return value;
}

constexpr uint64_t hton(uint64_t value) noexcept
{
	if constexpr (std::endian::native == std::endian::little)
		return bswap(value);
	else
		return value;
}


constexpr uint8_t ntoh(uint8_t value) noexcept
{
	return value;
}

constexpr uint16_t ntoh(uint16_t value) noexcept
{
	if constexpr (std::endian::native == std::endian::little)
		return bswap(value);
	else
		return value;
}

constexpr uint32_t ntoh(uint32_t value) noexcept
{
	if constexpr (std::endian::native == std::endian::little)
		return bswap(value);
	else
		return value;
}

constexpr uint64_t ntoh(uint64_t value) noexcept
{
	if constexpr (std::endian::native == std::endian::little)
		return bswap(value);
	else
		return value;
}


}


#endif
