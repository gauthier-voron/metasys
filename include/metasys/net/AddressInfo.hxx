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


#ifndef _INCLUDE_METASYS_NET_ADDRESSINFO_HXX_
#define _INCLUDE_METASYS_NET_ADDRESSINFO_HXX_


#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cstddef>


namespace metasys {


class AddressInfo
{
	struct addrinfo *_ais;


 public:
	constexpr AddressInfo() noexcept
		: _ais(NULL)
	{
	}

	explicit constexpr AddressInfo(struct addrinfo *ais) noexcept
		: _ais(ais)
	{
	}

	AddressInfo(const AddressInfo &other) = delete;
	AddressInfo(AddressInfo &&other) noexcept;
	~AddressInfo();

	AddressInfo &operator=(const AddressInfo &other) = delete;

	AddressInfo &operator=(AddressInfo &&other) noexcept
	{
		if (valid()) {
			if (other._ais != _ais) [[likely]]
				free();
		}

		_ais = other._ais;
		other._ais = NULL;

		return *this;
	}

	const struct addrinfo *value() const noexcept;

	constexpr bool valid() const noexcept
	{
		return (_ais != NULL);
	}

	struct addrinfo *reset(struct addrinfo *ais = NULL) noexcept;

	void free() noexcept;


	class Iterator
	{
		struct addrinfo *_ais;


	 public:
		constexpr Iterator() noexcept
			: _ais(NULL)
		{
		}

		constexpr Iterator(struct addrinfo *ais) noexcept
			: _ais(ais)
		{
		}

		constexpr Iterator(const Iterator &other) noexcept = default;
		Iterator(Iterator &&other) noexcept = default;

		Iterator &operator=(const Iterator &other) noexcept = default;
		Iterator &operator=(Iterator &&other) noexcept = default;

		Iterator &operator++() noexcept;
		Iterator operator++(int) noexcept;
		bool operator==(const Iterator &other) const noexcept;
		bool operator!=(const Iterator &other) const noexcept;
		struct addrinfo &operator*() const noexcept;
	};

	Iterator begin() noexcept;
	Iterator end() noexcept;


	class ConstIterator
	{
		const struct addrinfo *_ais;


	 public:
		constexpr ConstIterator() noexcept
			: _ais(NULL)
		{
		}

		constexpr ConstIterator(const struct addrinfo *ais) noexcept
			: _ais(ais)
		{
		}

		constexpr ConstIterator(const ConstIterator &other) noexcept =
			default;
		ConstIterator(ConstIterator &&other) noexcept = default;

		ConstIterator &operator=(const ConstIterator &other) noexcept =
			default;
		ConstIterator &operator=(ConstIterator &&other) noexcept =
			default;

		ConstIterator &operator++() noexcept;
		ConstIterator operator++(int) noexcept;
		bool operator==(const ConstIterator &other) const noexcept;
		bool operator!=(const ConstIterator &other) const noexcept;
		const struct addrinfo &operator*() const noexcept;
	};

	ConstIterator cbegin() const noexcept;
	ConstIterator cend() const noexcept;


	template<typename ErrHandler>
	auto resolve(const char *node, const char *service,
		     const struct addrinfo *hints, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);

		return handler(::getaddrinfo(node, service, hints, &_ais));
	}

	void resolve(const char *node, const char *service,
		     const struct addrinfo *hints);

	static void resolvethrow(int ret);
};


}


#endif
