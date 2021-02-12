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


#include <metasys/net/AddressInfo.hxx>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cstddef>

#include <metasys/net/ResolveException.hxx>
#include <metasys/sys/SystemException.hxx>


using metasys::AddressInfo;
using metasys::ResolveException;
using metasys::SystemException;


AddressInfo::AddressInfo(AddressInfo &&other) noexcept
	: _ais(other._ais)
{
	other._ais = NULL;
}

AddressInfo::~AddressInfo()
{
	::freeaddrinfo(_ais);
}

const struct addrinfo *AddressInfo::value() const noexcept
{
	return _ais;
}

struct addrinfo *AddressInfo::reset(struct addrinfo *ais) noexcept
{
	struct addrinfo *tmp = _ais;

	_ais = ais;

	return tmp;
}

void AddressInfo::free() noexcept
{
	::freeaddrinfo(_ais);
	_ais = NULL;
}

AddressInfo::Iterator &AddressInfo::Iterator::operator++() noexcept
{
	_ais = _ais->ai_next;
	return *this;
}

AddressInfo::Iterator AddressInfo::Iterator::operator++(int) noexcept
{
	Iterator ret = *this;
	++(*this);
	return ret;
}

bool AddressInfo::Iterator::operator==(const Iterator &other) const noexcept
{
	return (_ais == other._ais);
}

bool AddressInfo::Iterator::operator!=(const Iterator &other) const noexcept
{
	return !(*this == other);
}

struct addrinfo &AddressInfo::Iterator::operator*() const noexcept
{
	return *_ais;
}

AddressInfo::Iterator AddressInfo::begin() noexcept
{
	return Iterator(_ais);
}

AddressInfo::Iterator AddressInfo::end() noexcept
{
	return Iterator();
}

AddressInfo::ConstIterator &AddressInfo::ConstIterator::operator++() noexcept
{
	_ais = _ais->ai_next;
	return *this;
}

AddressInfo::ConstIterator AddressInfo::ConstIterator::operator++(int) noexcept
{
	ConstIterator ret = *this;
	++(*this);
	return ret;
}

bool AddressInfo::ConstIterator::operator==(const ConstIterator &other)
	const noexcept
{
	return (_ais == other._ais);
}

bool AddressInfo::ConstIterator::operator!=(const ConstIterator &other)
	const noexcept
{
	return !(*this == other);
}

const struct addrinfo &AddressInfo::ConstIterator::operator*() const noexcept
{
	return *_ais;
}

AddressInfo::ConstIterator AddressInfo::cbegin() const noexcept
{
	return ConstIterator(_ais);
}

AddressInfo::ConstIterator AddressInfo::cend() const noexcept
{
	return ConstIterator();
}

void AddressInfo::resolve(const char *node, const char *service,
			  const struct addrinfo *hints)
{
	resolve(node, service, hints, [](int ret) {
		if (ret != 0) [[unlikely]]
			resolvethrow(ret);
	});
}

void AddressInfo::resolvethrow(int ret)
{
	assert(ret != EAI_BADFLAGS);
	assert(ret != EAI_SOCKTYPE);

	switch (ret) {
	case EAI_ADDRFAMILY:
		throw ResolveException<EAI_ADDRFAMILY>();
	case EAI_AGAIN:
		throw ResolveException<EAI_AGAIN>();
	case EAI_FAIL:
		throw ResolveException<EAI_FAIL>();
	case EAI_FAMILY:
		throw ResolveException<EAI_FAMILY>();
	case EAI_MEMORY:
		throw ResolveException<EAI_MEMORY>();
	case EAI_NODATA:
		throw ResolveException<EAI_NODATA>();
	case EAI_NONAME:
		throw ResolveException<EAI_NONAME>();
	case EAI_SERVICE:
		throw ResolveException<EAI_SERVICE>();
	case EAI_SYSTEM:
		SystemException::throwErrno();
	}
}
