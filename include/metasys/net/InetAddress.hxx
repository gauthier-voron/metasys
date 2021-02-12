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


#ifndef _INCLUDE_METASYS_NET_INETADDRESS_HXX_
#define _INCLUDE_METASYS_NET_INETADDRESS_HXX_


#include <netinet/in.h>
#include <sys/types.h>

#include <cstdint>

#include <compare>

#include <metasys/net/bit.hxx>


namespace metasys {


class InetAddress
{
	struct sockaddr_in  _sin {};


 public:
	constexpr InetAddress() noexcept
	{
		_sin.sin_family = AF_INET;
	}

	constexpr InetAddress(uint16_t port) noexcept
	{
		_sin.sin_family = AF_INET;
		_sin.sin_port = hton(port);
		_sin.sin_addr.s_addr = hton(INADDR_ANY);
	}

	constexpr static InetAddress localhost(uint16_t port) noexcept
	{
		InetAddress ret;

		ret._sin.sin_family = AF_INET;
		ret._sin.sin_port = hton(port);
		ret._sin.sin_addr.s_addr = hton(INADDR_LOOPBACK);

		return ret;
	}

	constexpr InetAddress(const uint8_t ip[4], uint16_t port) noexcept
	{
		_sin.sin_family = AF_INET;
		_sin.sin_port = hton(port);
		_sin.sin_addr.s_addr =
			*(reinterpret_cast<const uint32_t *> (ip));
	}

	constexpr InetAddress(uint8_t ip0, uint8_t ip1, uint8_t ip2,
			      uint8_t ip3, uint16_t port) noexcept
	{
		_sin.sin_family = AF_INET;
		_sin.sin_port = hton(port);
		_sin.sin_addr.s_addr = hton((ip0 << 24) | (ip1 << 16)
					    | (ip2 << 8)
					    | static_cast<uint32_t> (ip3));
	}

	explicit constexpr InetAddress(const struct sockaddr_in *sin) noexcept
		: _sin(*sin)
	{
	}

	constexpr InetAddress(const InetAddress &other) noexcept = default;
	constexpr InetAddress(InetAddress &&other) noexcept = default;

	InetAddress &operator=(const InetAddress &other) noexcept = default;
	InetAddress &operator=(InetAddress &&other) noexcept = default;


	std::strong_ordering operator<=>(const InetAddress &other)
		const noexcept
	{
		size_t i;

		for (i = 0; i < 4; i++)
			if (auto cmp = (ip()[i] <=> other.ip()[i]); cmp != 0)
				return cmp;

		return (port() <=> other.port());
	}


	struct sockaddr_in *saddrin() noexcept
	{
		return &_sin;
	}

	constexpr const struct sockaddr_in *saddrin() const noexcept
	{
		return &_sin;
	}

	struct sockaddr *saddr() noexcept
	{
		return reinterpret_cast<sockaddr *> (&_sin);
	}

	const struct sockaddr *saddr() const noexcept
	{
		return reinterpret_cast<const sockaddr *> (&_sin);
	}

	constexpr static size_t size() noexcept
	{
		return sizeof (struct sockaddr_in);
	}


	constexpr uint16_t port() const noexcept
	{
		return ntoh(_sin.sin_port);
	}

	void port(uint16_t val) noexcept
	{
		_sin.sin_port = htons(val);
	}

	const uint8_t *ip() const noexcept
	{
		return reinterpret_cast<const uint8_t *>
			(&_sin.sin_addr.s_addr);
	}

	uint8_t *ip() noexcept
	{
		return reinterpret_cast<uint8_t *> (&_sin.sin_addr.s_addr);
	}
};


}


#endif
