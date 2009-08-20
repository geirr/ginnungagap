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

#ifndef GINNUNGAGAP_MIGRATIONGROUP_H
#define GINNUNGAGAP_MIGRATIONGROUP_H

#include "NetAddr.h"
#include "Uuid.h"

#include <list>
#include <map>

namespace ggg
{
	class MigrationGroup
	{
		public:
			MigrationGroup() : active_(true), newNotReadyGroup_(false) {}
			MigrationGroup(const Uuid& groupId) : active_(false), groupId_(groupId), newNotReadyGroup_(true) {}

			void addObject(const Uuid& objectId);
			void removeObject(const Uuid& objectId);

			void moveObjectToGroupWithMainObject(const Uuid& objectToMove, const Uuid& objectInNewGroup);
			void moveObjectToGroupWithMainObjectAfterConfimedDestination(const Uuid& objectToMove, const Uuid& objectInNewGroup);

			void addClientNeedObject(const NetAddr& netAddr, const Uuid& objectId);
			void addObjectNeedClient(const NetAddr& netAddr, const Uuid& objectId);
			void removeClientNeedObject(const Uuid& objectId, const NetAddr& netAddr);
			void removeObjectNeedClient(const Uuid& objectId, const NetAddr& netAddr);
			void removeClient(const NetAddr& netAddr);

			void migrateGroup(const NetAddr& destination);

			bool isActive() const;
			bool haveObject(const Uuid& objectId);
			Uuid groupId() const;
			void activateGroup();
			void tryToActivateIncoming(const Uuid& objectId);
			void waitFor(const Uuid& objectId);

			void doCoreSelection();

			void print() const;

			void forwardAllMsgsAndUpdateAllNs(const NetAddr& newHost);

		private:
			std::list<NetAddr> clientsThatNeedsThisObject(const Uuid& objectId) const;
			std::list<NetAddr> clientsThatThisObjectNeeds(const Uuid& objectId) const;

			std::list<NetAddr> clientsInGroup() const;

			/* Data */
			/* active = should be performed core sel on */
			bool active_;
			Uuid groupId_;

			std::list<Uuid> memberObjects_;

			/* Clients that have registerd that they need an object or more in this group */
			std::map< Uuid, std::list<NetAddr> > clientsThatNeedsThisObject_;

			/* Clients the objects in this group needs */
			std::map< Uuid, std::list<NetAddr> > clientsThatThisObjectNeed_;

			/* Only used when deactivated, don't have to serialize when a group migrates */
			std::list<Uuid> waitingFor_;
			std::list<Uuid> objectsToBeSent_;
			bool newNotReadyGroup_;
	};
}

#endif

