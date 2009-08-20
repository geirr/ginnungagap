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

#ifndef GINNUNGAGAP_SKELETON_H
#define GINNUNGAGAP_SKELETON_H

#include "Uuid.h"
#include "MessageType.h"

namespace ggg
{
	class XdrSendBuffer;
	class Object;
	class XdrReceiveBuffer;

	class Skeleton
	{
		public:
			Skeleton() : rmiRetMsg_(RMIACKRETVAL), ackType_(RMIOK) {}
			XdrSendBuffer* makeRmiRetMsg(const int& size = 0);
			XdrSendBuffer* handleRmi(const int& objectName, const Uuid& objectId, Object* object, XdrReceiveBuffer* xdrReciveBuffer, const bool& piggybackNetAddr = false);
			void handleEvent(const int& objectName, const Uuid& objectId, Object* object, XdrReceiveBuffer* xdrReciveBuffer);
		private:
			int funcNr_;
			int rmiRetMsg_;
			int ackType_;
			Uuid objectId_;
			bool piggybackNetAddr_;

	};
}

#endif

