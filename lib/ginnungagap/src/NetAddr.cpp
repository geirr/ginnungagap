/************************************************************************
*                                                                       *
*  Copyright (C) 2009 by Geir Erikstad                                  *
*  geirr@baldr.no                                                       *
*                                                                       *
*  This file is part of Ginnungagap.                                    *
*                                                                       *
*  Ginnungagap is free software: you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by *
*  the Free Software Foundation, either version 2 of the License, or    *
*  (at your option) any later version.                                  *
*                                                                       *
*  Ginnungagap is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
*  GNU General Public License for more details.                         *
*                                                                       *
*  You should have received a copy of the GNU General Public License    *
*  along with Ginnungagap. If not, see <http://www.gnu.org/licenses/>.  *
*                                                                       *
************************************************************************/

#include "NetAddr.h"

#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

using std::string;
using std::cerr;
using std::endl;

namespace ginnungagap
{
	NetAddr::NetAddr(const std::string& ip, const std::string& port)
	{
		initFromStr(ip, port);
	}

	NetAddr::NetAddr(const std::string& addr)
	{
		string::size_type pos = addr.find(":");
		string ip = addr.substr(0, pos);
		string port = addr.substr(pos+1, addr.size());
		initFromStr(ip, port);
	}

	void NetAddr::initFromStr(const std::string& ip, const std::string& port)
	{
		/*
		struct in_addr addr;
		inet_pton(AF_INET, ip.c_str(), &(addr.s_addr));
		ip_ = addr.s_addr;

		std::stringstream portss(port);
		unsigned short port_h;
		portss >> port_h;
		port_ = htons(port_h);
		*/

		struct addrinfo hints;
		struct addrinfo* res;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		int retVal;
		if ((retVal = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res)) != 0)
		{
			cerr << "error: getaddrinfo: " << gai_strerror(retVal) << endl;
		}

		struct sockaddr_in* sockaddr = (sockaddr_in *) res->ai_addr;
		ip_ = sockaddr->sin_addr.s_addr;
		port_ = sockaddr->sin_port;

		freeaddrinfo(res);
	}

	NetAddr::NetAddr(const unsigned long& ip, const unsigned short& port)
	{
		ip_ = ip;
		port_ = port;
	}

	NetAddr::NetAddr(const struct sockaddr_in& sockaddr)
	{
		ip_ = sockaddr.sin_addr.s_addr;
		port_ = sockaddr.sin_port;
	}

	void NetAddr::setAddress(const struct sockaddr_in& sockaddr)
	{
		ip_ = sockaddr.sin_addr.s_addr;
		port_ = sockaddr.sin_port;
	}

	std::string NetAddr::addressString() const
	{
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ip_, str, INET_ADDRSTRLEN);

		std::stringstream address_ss;
		address_ss << str;
		address_ss << ":";
		address_ss << ntohs(port_);

		return address_ss.str();
	}

	struct sockaddr_in* NetAddr::address() const
	{
		struct sockaddr_in* sockaddr = new struct sockaddr_in;
		sockaddr->sin_family = AF_INET;
		sockaddr->sin_port = port_;
		sockaddr->sin_addr.s_addr = ip_;
		return sockaddr;
	}

	string NetAddr::port() const
	{
		std::stringstream ss;
		ss << ntohs(port_);
		return ss.str();
	}

	bool NetAddr::operator==(const NetAddr& netAddr) const
	{
		struct sockaddr_in* sockaddr = netAddr.address();
		bool ret = (sockaddr->sin_addr.s_addr == ip_ && sockaddr->sin_port == port_);
		delete sockaddr;
		return ret;
	}

	bool NetAddr::operator<(const NetAddr& netAddr) const
	{
		struct sockaddr_in* sockaddr = netAddr.address();
		bool ret = (sockaddr->sin_addr.s_addr < ip_ || sockaddr->sin_port < port_);
		delete sockaddr;
		return ret;
	}
}

