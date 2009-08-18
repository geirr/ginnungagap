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

#ifndef GINNUNGAGAP_MESSAGEHANDLER_H
#define GINNUNGAGAP_MESSAGEHANDLER_H

#include <set>

namespace ginnungagap
{
	class XdrReceiveBuffer;
	struct Message;

	class MessageHandler
	{
		public:
			MessageHandler() : verbose_(false) {}
			void handleMessage(Message* message);

		private:
			void handleInit(XdrReceiveBuffer* xdrBuffer);
			void handleInitAck(XdrReceiveBuffer* xdrBuffer);
			void handleMainInit(XdrReceiveBuffer* xdrBuffer);
			void handleProxyInfo(XdrReceiveBuffer* xdrBuffer);
			void handleProxyInit(XdrReceiveBuffer* xdrBuffer);
			void handleNeedObject(XdrReceiveBuffer* xdrBuffer);
			void handleDontNeedObject(XdrReceiveBuffer* xdrBuffer);
			void handleRmi(XdrReceiveBuffer* xdrBuffer);
			void handleEvent(XdrReceiveBuffer* xdrBuffer);
			void handleForwardedRmi(XdrReceiveBuffer* xdrBuffer);
			void handleRmiAckAndReturnValue(XdrReceiveBuffer* xdrBuffer);
			void handleIncomingMigration(XdrReceiveBuffer* xdrBuffer);
			void handleMigrationAck(XdrReceiveBuffer* xdrBuffer);
			void handleNameServiceUpdate(XdrReceiveBuffer* xdrBuffer);
			void handleLatencyInformation(XdrReceiveBuffer* xdrBuffer);
			void handleMigrationGroup(XdrReceiveBuffer* xdrBuffer);
			void handleMigrationGroupSentAll(XdrReceiveBuffer* xdrBuffer);
			void handleMigrationGroupAck(XdrReceiveBuffer* xdrBuffer);
			void handleInitChangeObjectGroup(XdrReceiveBuffer* xdrBuffer);
			void handleICOGNotHere(XdrReceiveBuffer* xdrBuffer);
			void handleICOGIsHere(XdrReceiveBuffer* xdrBuffer);
			void handleGroupInfoForObject(XdrReceiveBuffer* xdrBuffer);

			Message* currentMessage_;

			const bool verbose_;
	};
}

#endif

