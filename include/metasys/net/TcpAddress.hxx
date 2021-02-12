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


#ifndef _INCLUDE_METASYS_NET_TCPADDRESS_HXX_
#define _INCLUDE_METASYS_NET_TCPADDRESS_HXX_


#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <utility>
#include <string>

#include <metasys/net/AddressInfo.hxx>
#include <metasys/net/InetAddress.hxx>


namespace metasys {


class TcpAddress
{
 public:
	template<typename ErrHandler>
	static auto resolve(AddressInfo *dest, const char *node,
			    const char *service, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		struct addrinfo hints;

		hints.ai_flags = 0;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		return dest->resolve(node, service, &hints,
				     std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	static auto resolve(AddressInfo *dest, const std::string &node,
			    const std::string &service, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return resolve(dest, node.c_str(), service.c_str(),
			       std::forward<ErrHandler>(handler));
	}


	template<typename ErrHandler>
	static InetAddress instance(const char *node, const char *service,
				    ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		AddressInfo ais;
		int ret;

		if ((ret = resolve(&ais, node, service, [](int ret) {
			return ret;
		})) != 0) [[unlikely]]
			goto err;

		for (const struct addrinfo &ai : ais) {
			assert(ai.ai_family == AF_INET);
			assert(ai.ai_socktype == SOCK_STREAM);
			assert(ai.ai_protocol == IPPROTO_TCP);
			assert(ai.ai_addrlen == InetAddress::size());

			return InetAddress
				(reinterpret_cast<const struct sockaddr_in *>
				 (ai.ai_addr));
		}

		ret = EAI_SERVICE;
	 err:
		handler(ret);
		return InetAddress();
	}

	template<typename ErrHandler>
	static InetAddress instance(const std::string &node,
				    const std::string &service,
				    ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return instance(node.c_str(), service.c_str(),
				std::forward<ErrHandler>(handler));
	}

	template<typename ErrHandler>
	static InetAddress instance(const char *node, uint16_t port,
				    ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		AddressInfo ais;
		int ret;

		if ((ret = resolve(&ais, node, NULL, [](int ret) {
			return ret;
		})) != 0) [[unlikely]]
			goto err;

		for (struct addrinfo &ai : ais) {
			assert(ai.ai_family == AF_INET);
			assert(ai.ai_socktype == SOCK_STREAM);
			assert(ai.ai_protocol == IPPROTO_TCP);
			assert(ai.ai_addrlen == InetAddress::size());

			(reinterpret_cast<struct sockaddr_in *> (ai.ai_addr))
				->sin_port = htons(port);

			return InetAddress
				(reinterpret_cast<struct sockaddr_in *>
				 (ai.ai_addr));
		}

		ret = EAI_SERVICE;
	 err:
		handler(ret);
		return InetAddress();
	}

	template<typename ErrHandler>
	static InetAddress instance(const std::string &node, uint16_t port,
				    ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		return instance(node.c_str(), port,
				std::forward<ErrHandler>(handler));
	}

	static InetAddress instance(const char *node, const char *service)
	{
		return instance(node, service, [](int ret) {
			if (ret != 0) [[unlikely]]
				AddressInfo::resolvethrow(ret);
		});
	}

	static InetAddress instance(const std::string &node,
				    const std::string &service)
	{
		return instance(node.c_str(), service.c_str());
	}

	static InetAddress instance(const char *node, uint16_t port)
	{
		return instance(node, port, [](int ret) {
			if (ret != 0) [[unlikely]]
				AddressInfo::resolvethrow(ret);
		});
	}

	static InetAddress instance(const std::string &node, uint16_t port)
	{
		return instance(node.c_str(), port);
	}
};


}


#endif
