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

#include "MigrationService.h"
#include "MigrationGroup.h"
#include "XdrBuffer.h"
#include "XdrSendBuffer.h"
#include "Ginnungagap.h"
#include "NameService.h"
#include "CommunicationSocketManager.h"
#include "Object.h"

#include <iostream>

using std::cerr;
using std::endl;

using std::list; using std::map; using std::queue; using std::string;

namespace ggg
{
	typedef map< Uuid, queue<XdrBuffer*>* >::iterator bufferQueueMapItr_t;
	typedef list<MigrationGroup*>::const_iterator cMigGroupsItr_T;
	typedef list<MigrationGroup*>::iterator migGroupsItr_T;

	void MigrationService::migrateObject(const Uuid& objectId, const NetAddr& netAddr)
	{
		Object* object = Ginnungagap::Instance()->nameService()->getLocalObjectOrProxy(objectId);
		object->setObjectState(Object::MIGRATING);
		XdrSendBuffer* xdr = object->deflate();
		Ginnungagap::Instance()->sendXdrBuffer(netAddr, xdr);
		string logMsg = "Migrating object ";
		logMsg += objectId.getHexString();
		logMsg += " to node ";
		logMsg += netAddr.addressString();
		Ginnungagap::Instance()->writeToLog(logMsg);
	}

	XdrSendBuffer* MigrationService::getDeflatedObject(const Uuid& objectId)
	{
		Object* object = Ginnungagap::Instance()->nameService()->getLocalObjectOrProxy(objectId);
		object->setObjectState(Object::MIGRATING);
		return object->deflate();
	}
	
	void MigrationService::putXdrBufferInQueue(const Uuid& objectId, XdrBuffer* xdrBuffer)
	{
		bufferQueueMapItr_t bufferQueueMapItr = bufferQueueMap_.find(objectId);
		if (bufferQueueMapItr == bufferQueueMap_.end())
		{
			queue<XdrBuffer*>* bufferQueue = new queue<XdrBuffer*>;
			bufferQueue->push(xdrBuffer);
			bufferQueueMap_[objectId] = bufferQueue;
		}
		else
		{
			queue<XdrBuffer*>* bufferQueue = bufferQueueMapItr->second;
			bufferQueue->push(xdrBuffer);
		}
	}

	void MigrationService::updateNameServiceAndForwardRmiMessages(const NetAddr& netAddr, const Uuid& objectId)
	{
		Ginnungagap::Instance()->nameService()->setAsRemote(objectId, netAddr);
		bufferQueueMapItr_t bufferQueueMapItr = bufferQueueMap_.find(objectId);
		if (bufferQueueMapItr != bufferQueueMap_.end())
		{
			queue<XdrBuffer*>* bufferQueue = bufferQueueMapItr->second;

			while (!(bufferQueue->empty()))
			{
				Ginnungagap::Instance()->sendXdrBuffer(netAddr, bufferQueue->front());
				bufferQueue->pop();
			}
		}
	}

	MigrationGroup* MigrationService::getMigrationGroup(const Uuid& objectId)
	{
		for (cMigGroupsItr_T itr = migrationGroups_.begin(); itr != migrationGroups_.end(); ++itr)
		{
			if ((*itr)->haveObject(objectId))
				return *itr;
		}
		return 0;
	}

	MigrationGroup* MigrationService::getMigrationGroupById(const Uuid& groupId)
	{
		for (cMigGroupsItr_T itr = migrationGroups_.begin(); itr != migrationGroups_.end(); ++itr)
		{
			if ((*itr)->groupId() == groupId)
				return *itr;
		}
		return 0;
	}

	void MigrationService::addMigrationGroup(MigrationGroup* migrationGroup)
	{
		migrationGroups_.push_back(migrationGroup);
	}

	void MigrationService::deleteMigrationGroup(const Uuid& groupId)
	{
		for (migGroupsItr_T itr = migrationGroups_.begin(); itr != migrationGroups_.end(); ++itr)
		{
			if ((*itr)->groupId() == groupId)
			{
				MigrationGroup* mg = *itr;
				delete mg;
				migrationGroups_.erase(itr);
				return;
			}
		}
	}

	void MigrationService::doCoreSelectionOnAllGroups()
	{
		string logMsg = "Initiating core selection";
		Ginnungagap::Instance()->writeToLog(logMsg);
		for (migGroupsItr_T migGroup = migrationGroups_.begin(); migGroup != migrationGroups_.end(); ++ migGroup)
		{
			(*migGroup)->doCoreSelection();
		}
	}

	void MigrationService::deleteClientFromAllGroups(const NetAddr& netAddr)
	{
		for (migGroupsItr_T migGroup = migrationGroups_.begin(); migGroup != migrationGroups_.end(); ++ migGroup)
		{
			(*migGroup)->removeClient(netAddr);
		}
	}
}

