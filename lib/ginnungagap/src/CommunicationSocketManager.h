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

#ifndef GINNUNGAGAP_COMMUNICATIONSOCKETMANAGER_H
#define GINNUNGAGAP_COMMUNICATIONSOCKETMANAGER_H

#include "NetAddr.h"

#include <map>

namespace ggg
{
	class CommunicationSocket;

	class CommunicationSocketManager
	{
		public:
			CommunicationSocket* getCommunicationSocket(const NetAddr& netAddr);
			void addSocket(CommunicationSocket* commSock);
			void removeSocket(const CommunicationSocket* commSock);
			bool exists(const NetAddr& netAddr);

		private:
			CommunicationSocket* openNewConnection(const NetAddr& netAddr);
			std::map<NetAddr, CommunicationSocket*> socketRegister_;
	};
}

#endif

