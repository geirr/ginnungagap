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

#ifndef GINNUNGAGAP_PROXY_H
#define GINNUNGAGAP_PROXY_H

#include "MessageType.h"

namespace ggg
{
	class XdrReceiveBuffer;
	class XdrSendBuffer;
	class Object;

	class Proxy
	{
		protected:
			Proxy() : rmi_(RMI), event_(EVENT), stdSize_(INT*3 + OBJID)  {}
			void sendNeed() const;
			void sendDontNeed() const;
			XdrReceiveBuffer* sendAndGetRetVal(XdrSendBuffer* xdrBuffer);
			void sendEvent(XdrSendBuffer* xdrBuffer);

			XdrSendBuffer* makeRmiMsg(int funcNr, const int& size = 0);
			XdrSendBuffer* makeEventMsg(int funcNr, const int& size = 0);

			int objectType_;
			int rmi_;
			int event_;
			int stdSize_;
			Object* object_;
	};
}

#endif

