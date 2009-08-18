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

#ifndef GINNUNGAGAP_NETADDR_H
#define GINNUNGAGAP_NETADDR_H

#include <string>
#include <netinet/in.h>

namespace ginnungagap
{
	class NetAddr
	{
		public:
			NetAddr() : ip_(0), port_(0) {}
			NetAddr(const std::string& ip, const std::string& port);
			NetAddr(const std::string& addr);
			NetAddr(const unsigned long& ip, const unsigned short& port);
			NetAddr(const struct sockaddr_in& sockaddr);

			std::string addressString() const;
			struct sockaddr_in* address() const;
			void setAddress(const struct sockaddr_in& sockaddr);

			std::string port() const;

			bool operator==(const NetAddr& netAddr) const;
			bool operator<(const NetAddr& netAddr) const;
		private:
			//Addresses in network byte order
			void initFromStr(const std::string& ip, const std::string& port);
			unsigned long ip_;
			unsigned short port_;
	};
}

#endif

