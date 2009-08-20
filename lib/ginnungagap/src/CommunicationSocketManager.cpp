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

#include "CommunicationSocketManager.h"
#include "CommunicationSocket.h"

#include <iostream>

using std::cerr;
using std::endl;

namespace ggg
{
	typedef std::map<NetAddr, CommunicationSocket*>::iterator regItr_t;
	typedef std::map<NetAddr, CommunicationSocket*>::const_iterator cRegItr_t;

	CommunicationSocket* CommunicationSocketManager::getCommunicationSocket(const NetAddr& netAddr)
	{
		regItr_t regItr = socketRegister_.find(netAddr);
		if (regItr != socketRegister_.end())
		{
			return regItr->second;
		}
		else
		{
			return openNewConnection(netAddr);
		}
	}

	void CommunicationSocketManager::addSocket(CommunicationSocket* commSock)
	{
		socketRegister_[commSock->otherEnd()] = commSock;
	}

	void CommunicationSocketManager::removeSocket(const CommunicationSocket* commSock)
	{
		regItr_t regItr = socketRegister_.find(commSock->otherEnd());
		if (regItr != socketRegister_.end())
		{
			socketRegister_.erase(regItr);
		}
	}

	CommunicationSocket* CommunicationSocketManager::openNewConnection(const NetAddr& netAddr)
	{
		CommunicationSocket* commSock = new CommunicationSocket(netAddr);
		return commSock;
	}

	bool CommunicationSocketManager::exists(const NetAddr& netAddr)
	{
		cRegItr_t regItr = socketRegister_.find(netAddr);
		if (regItr == socketRegister_.end())
			return false;
		return true;
	}
}

