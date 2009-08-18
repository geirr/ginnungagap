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

#include "ListenSocket.h"
#include "CommunicationSocket.h"

#include <string>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using std::string;
using std::cerr;
using std::endl;

namespace ginnungagap
{
	ListenSocket::ListenSocket(string port, int queueLength)
	{
		struct addrinfo hints;
		struct addrinfo* res;
		int yes = 1;
		int retVal;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		if ((retVal = getaddrinfo(0, port.c_str(), &hints, &res)) != 0)
		{
			cerr << "Error: getaddrinfo: " << gai_strerror(retVal) << endl;
		}
		
		if ((socketDescriptor_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		{
			cerr << "Error: socket" << endl;
		}

		if (setsockopt(socketDescriptor_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			cerr << "Error: setsockopt" << endl;
		}

		if (bind(socketDescriptor_, res->ai_addr, res->ai_addrlen) == -1)
		{
			cerr << "Error: bind" << endl;
		}

		if (listen(socketDescriptor_, queueLength) == -1)
		{
			cerr << "Error: listen" << endl;
		}

		freeaddrinfo(res);
	}
	CommunicationSocket* ListenSocket::acceptConnection()
	{
		int newSocketDescriptor;
		newSocketDescriptor = accept(socketDescriptor_, 0, 0);
		return new CommunicationSocket(newSocketDescriptor); 
	}
}

