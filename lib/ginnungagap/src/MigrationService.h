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

#ifndef GINNUNGAGAP_MIGRATIONSERVICE_H
#define GINNUNGAGAP_MIGRATIONSERVICE_H

#include "NetAddr.h"
#include "Uuid.h"

#include <map>
#include <queue>
#include <list>

namespace ggg
{
	class MigrationGroup;
	class XdrBuffer;
	class XdrSendBuffer;

	class MigrationService
	{
		public:
			void addMigrationGroup(MigrationGroup* migrationGroup);
			void deleteMigrationGroup(const Uuid& groupId);

			void deleteClientFromAllGroups(const NetAddr& netAddr);

			MigrationGroup* getMigrationGroup(const Uuid& objectId);
			MigrationGroup* getMigrationGroupById(const Uuid& groupId);

			void migrateObject(const Uuid& objectId, const NetAddr& netAddr);

		private:
			friend class Ginnungagap;
			friend class MessageHandler;
			friend class MigrationGroup;
			friend class Proxy;
			void putXdrBufferInQueue(const Uuid& objectId, XdrBuffer* xdrBuffer);
			void updateNameServiceAndForwardRmiMessages(const NetAddr& netAddr, const Uuid& objectId);
			void doCoreSelectionOnAllGroups();

			/* Called by mig grp to be able to pack all objs in a single msg */
			XdrSendBuffer* getDeflatedObject(const Uuid& objectId);

			/* buffers with rmi msg to forward after finished migration */
			std::map< Uuid, std::queue<XdrBuffer*>* > bufferQueueMap_;

			std::list<MigrationGroup*> migrationGroups_;
	};
}

#endif

