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

#include "MigrationGroup.h"
#include "Ginnungagap.h"
#include "XdrSendBuffer.h"
#include "MigrationService.h"
#include "NetworkGraph.h"
#include "NameService.h"
#include "Object.h"

#include <sstream>

using std::pair; using std::make_pair; using std::list; using std::map; using std::stringstream;

using std::cerr; using std::endl;

namespace ginnungagap
{
	typedef list<Uuid>::const_iterator cObjItr_t;
	typedef list<Uuid>::iterator objItr_t;
	typedef map< Uuid, list<NetAddr> >::iterator objIdToClientsItr_t;
	typedef map< Uuid, list<NetAddr> >::const_iterator cObjIdToClientsItr_t;
	typedef list<NetAddr>::iterator netAddrListItr_t;
	typedef list<NetAddr>::const_iterator cNetAddrListItr_t;

	void MigrationGroup::addObject(const Uuid& objectId)
	{
		/* don't do anything if its already there */
		for (objItr_t object = memberObjects_.begin(); object != memberObjects_.end(); ++object)
		{
			if (*object == objectId)
			{
				return;
			}
		}
		memberObjects_.push_back(objectId);
		//stringstream logMsg;
		//logMsg << "MigrationGroup: Adding object  " << objectId.getHexString() << " to group " << groupId_.getHexString();
		//Ginnungagap::Instance()->writeToLog(logMsg.str());
	}

	void MigrationGroup::moveObjectToGroupWithMainObject(const Uuid& objectToMove, const Uuid& objectInNewGroup)
	{
		stringstream logMsg;
		logMsg << "MigrationGroup: Moving object  " << objectToMove.getHexString() << " to group with object " << objectInNewGroup.getHexString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());
		if (Ginnungagap::Instance()->nameService()->isLocal(objectInNewGroup))
		{
			/* The objectInNewGroup might be migrating to another server...
			 * Will need to do something with this later, for now, just catch the error */
			if (Ginnungagap::Instance()->nameService()->getLocalObjectOrProxy(objectInNewGroup)->objectState() == Object::MIGRATING)
			{
				cerr << "Error: trying to move object to local group which is migrating..." << endl;
				exit(1);
			}

			list<NetAddr> needObject = clientsThatNeedsThisObject(objectToMove);
			list<NetAddr> objectNeed = clientsThatThisObjectNeeds(objectToMove);
			removeObject(objectToMove);
			Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectInNewGroup)->addObject(objectToMove);

			for (cNetAddrListItr_t nalItr = needObject.begin(); nalItr != needObject.end(); ++ nalItr)
			{
				Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectInNewGroup)->addClientNeedObject(*nalItr, objectToMove);
			}

			for (cNetAddrListItr_t nalItr = objectNeed.begin(); nalItr != objectNeed.end(); ++ nalItr)
			{
				Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectInNewGroup)->addObjectNeedClient(*nalItr, objectToMove);
			}
		}
		else
		{
			active_ = false;
			objectsToBeSent_.push_back(objectToMove);
			/* get NetAddr to node with group we want to move to */
			NetAddr destination = Ginnungagap::Instance()->nameService()->netAddr(objectInNewGroup);
			/* send a msg to the dest node */
			int msg = INITCHOBJGRP;
			XdrSendBuffer* xdr = new XdrSendBuffer(INT + OBJID*2);
			*xdr << msg << objectToMove << objectInNewGroup;
			Ginnungagap::Instance()->sendXdrBuffer(destination, xdr);
		}
	}

	void MigrationGroup::moveObjectToGroupWithMainObjectAfterConfimedDestination(const Uuid& objectToMove, const Uuid& objectInNewGroup)
	{
		/* send info about object, and the object itself, then activate this group again */
		list<NetAddr> needObject = clientsThatNeedsThisObject(objectToMove);
		list<NetAddr> objectNeed = clientsThatThisObjectNeeds(objectToMove);

		int numberOfNetAddrs = needObject.size() + objectNeed.size();

		XdrSendBuffer* xdr = new XdrSendBuffer(INT*3 + OBJID*2 + NETADDR*numberOfNetAddrs);

		int msg = OBJGRPINFO;

		*xdr << msg << objectToMove << objectInNewGroup;

		int tmpSize;
		tmpSize = needObject.size();
		*xdr << tmpSize;
		for (cNetAddrListItr_t nalItr = needObject.begin(); nalItr != needObject.end(); ++ nalItr)
		{
			*xdr << *nalItr;
		}

		tmpSize = objectNeed.size();
		*xdr << tmpSize;
		for (cNetAddrListItr_t nalItr = objectNeed.begin(); nalItr != objectNeed.end(); ++ nalItr)
		{
			*xdr << *nalItr;
		}

		Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->nameService()->netAddr(objectInNewGroup), xdr);
		removeObject(objectToMove);
		Ginnungagap::Instance()->migrationService()->migrateObject(objectToMove, Ginnungagap::Instance()->nameService()->netAddr(objectInNewGroup));

		objectsToBeSent_.remove(objectToMove);

		if (objectsToBeSent_.empty() && waitingFor_.empty())
		{
			active_ = true;
		}
	}

	void MigrationGroup::removeObject(const Uuid& objectId)
	{
		clientsThatThisObjectNeed_.erase(objectId);
		clientsThatNeedsThisObject_.erase(objectId);
		for (objItr_t object = memberObjects_.begin(); object != memberObjects_.end(); ++object)
		{
			if (*object == objectId)
			{
				memberObjects_.erase(object);
				stringstream logMsg;
				logMsg << "MigrationGroup: Removing object  " << objectId.getHexString() << " from group " << groupId_.getHexString();
				Ginnungagap::Instance()->writeToLog(logMsg.str());
				return;
			}
		}
	}

	void MigrationGroup::addClientNeedObject(const NetAddr& netAddr, const Uuid& objectId)
	{
		clientsThatNeedsThisObject_[objectId].push_back(netAddr);
		clientsThatNeedsThisObject_[objectId].sort();
		clientsThatNeedsThisObject_[objectId].unique();
	}

	void MigrationGroup::addObjectNeedClient(const NetAddr& netAddr, const Uuid& objectId)
	{
		clientsThatThisObjectNeed_[objectId].push_back(netAddr);
		clientsThatThisObjectNeed_[objectId].sort();
		clientsThatThisObjectNeed_[objectId].unique();
	}

	void MigrationGroup::removeClient(const NetAddr& netAddr)
	{
		for (objIdToClientsItr_t ocItr = clientsThatThisObjectNeed_.begin(); ocItr != clientsThatThisObjectNeed_.end(); ++ocItr)
		{
			ocItr->second.remove(netAddr);
		}

		for (objIdToClientsItr_t ocItr = clientsThatNeedsThisObject_.begin(); ocItr != clientsThatNeedsThisObject_.end(); ++ocItr)
		{
			ocItr->second.remove(netAddr);
		}
	}

	void MigrationGroup::removeClientNeedObject(const Uuid& objectId, const NetAddr& netAddr)
	{
		objIdToClientsItr_t itr = clientsThatNeedsThisObject_.find(objectId);
		if (itr != clientsThatNeedsThisObject_.end())
		{
			itr->second.remove(netAddr);
		}
	}

	void MigrationGroup::removeObjectNeedClient(const Uuid& objectId, const NetAddr& netAddr)
	{
		objIdToClientsItr_t itr = clientsThatThisObjectNeed_.find(objectId);
		if (itr != clientsThatThisObjectNeed_.end())
		{
			itr->second.remove(netAddr);
		}
	}

	bool MigrationGroup::haveObject(const Uuid& objectId)
	{
		for (cObjItr_t object = memberObjects_.begin(); object != memberObjects_.end(); ++object)
		{
			if (*object == objectId)
				return true;
		}
		return false;
	}

	bool MigrationGroup::isActive() const
	{
		return active_;
	}

	void MigrationGroup::migrateGroup(const NetAddr& destination)
	{
		stringstream logMsg;
		logMsg << "MigrationGroup: Migrating group " << groupId_.getHexString() << " to node " << destination.addressString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());

		active_ = false;

		/* get size of xdr buffer */
		int numberOfUuids = 0;
		int numberOfNetAddrs = 0;
		int numberOfints = 3;

		numberOfUuids += memberObjects_.size();
		numberOfUuids += clientsThatThisObjectNeed_.size();
		for (objIdToClientsItr_t ocItr = clientsThatThisObjectNeed_.begin(); ocItr != clientsThatThisObjectNeed_.end(); ++ocItr)
		{
			numberOfNetAddrs += ocItr->second.size();
			++numberOfints;
		}

		numberOfUuids += clientsThatNeedsThisObject_.size();
		for (objIdToClientsItr_t ocItr = clientsThatNeedsThisObject_.begin(); ocItr != clientsThatNeedsThisObject_.end(); ++ocItr)
		{
			numberOfNetAddrs += ocItr->second.size();
			++numberOfints;
		}

		XdrSendBuffer* xdr = new XdrSendBuffer(INT + UUID + numberOfints*INT + numberOfUuids*UUID + numberOfNetAddrs*NETADDR);

		/* fill the xdr buffer */
		int msgType = MIGGRP;
		*xdr << msgType << groupId_;

		int tmpSize;

		tmpSize = memberObjects_.size();
		*xdr << tmpSize;
		for (objItr_t objItr = memberObjects_.begin(); objItr != memberObjects_.end(); ++objItr)
		{
			*xdr << *objItr;
		}

		tmpSize = clientsThatNeedsThisObject_.size();
		*xdr << tmpSize;
		for (objIdToClientsItr_t ocItr = clientsThatNeedsThisObject_.begin(); ocItr != clientsThatNeedsThisObject_.end(); ++ocItr)
		{
			*xdr << ocItr->first;
			tmpSize = ocItr->second.size();
			*xdr << tmpSize;
			for (netAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				*xdr << *nalItr;
			}
		}

		tmpSize = clientsThatThisObjectNeed_.size();
		*xdr << tmpSize;
		for (objIdToClientsItr_t ocItr = clientsThatThisObjectNeed_.begin(); ocItr != clientsThatThisObjectNeed_.end(); ++ocItr)
		{
			*xdr << ocItr->first;
			tmpSize = ocItr->second.size();
			*xdr << tmpSize;
			for (netAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				*xdr << *nalItr;
			}
		}

		/* Get the objects */
		list<XdrSendBuffer*> objects;
		int totalSize = xdr->bufferSize();
		for (objItr_t objItr = memberObjects_.begin(); objItr != memberObjects_.end(); ++objItr)
		{
			XdrSendBuffer* deflatedObject = Ginnungagap::Instance()->migrationService()->getDeflatedObject(*objItr);
			totalSize += (deflatedObject->bufferSize() - INT);
			objects.push_back(Ginnungagap::Instance()->migrationService()->getDeflatedObject(*objItr));
		}

		/* Create one big xdr buffer with groupinfo and objects */
		XdrSendBuffer* wholeGroup = new XdrSendBuffer(totalSize);

		/* Copy groupinfo to the start */
		wholeGroup->insert(xdr);
		delete xdr;

		/* Copy each of the objects into the buffer */
		for(list<XdrSendBuffer*>::iterator objectXdrItr = objects.begin(); objectXdrItr != objects.end(); ++objectXdrItr)
		{
			wholeGroup->insert(*objectXdrItr, INT);
			delete *objectXdrItr;
		}

		/* send the group */
		//logMsg.str("");
		//logMsg << "MigrationGroup: Sending grp now";
		//Ginnungagap::Instance()->writeToLog(logMsg.str());
		Ginnungagap::Instance()->sendXdrBuffer(destination, wholeGroup);
	}

	Uuid MigrationGroup::groupId() const
	{
		return groupId_;
	}

	void MigrationGroup::activateGroup()
	{
		active_ = true;
		newNotReadyGroup_ = false;
	}

	void MigrationGroup::tryToActivateIncoming(const Uuid& objectId)
	{
		for (objItr_t objIdItr = waitingFor_.begin(); objIdItr != waitingFor_.end(); ++objIdItr)
		{
			if (*objIdItr == objectId)
			{
				if (haveObject(objectId) && Ginnungagap::Instance()->nameService()->isLocal(objectId))
				{
					waitingFor_.remove(objectId);
					if (!newNotReadyGroup_ && waitingFor_.empty() && objectsToBeSent_.empty())
					{
						active_ = true;
					}
				}
				return;
			}
		}
	}

	void MigrationGroup::waitFor(const Uuid& objectId)
	{
		active_ = false;
		waitingFor_.push_back(objectId);

		/* just to be shure: */
		waitingFor_.sort();
		waitingFor_.unique();
	}

	void MigrationGroup::doCoreSelection()
	{
		if (!active_)
			return;
		stringstream logMsg;
		logMsg << "MigrationGroup: Group " << groupId_.getHexString() << " performs core selection:";
		Ginnungagap::Instance()->writeToLog(logMsg.str());
		list<NetAddr> clients = clientsInGroup();
		pair<NetAddr, double> bestNode = Ginnungagap::Instance()->networkGraph()->findBestCore(clients);
		logMsg.str("");
		logMsg << "MigrationGroup: Best location is ";
		logMsg << bestNode.first.addressString() << " and the gain would be " << bestNode.second;
		Ginnungagap::Instance()->writeToLog(logMsg.str());
		if (!(bestNode.first == Ginnungagap::Instance()->netAddr()) && (bestNode.second >= Ginnungagap::Instance()->coreSelectionThreshold()))
		{
			migrateGroup(bestNode.first);
		}
	}

	list<NetAddr> MigrationGroup::clientsThatNeedsThisObject(const Uuid& objectId) const
	{
		cObjIdToClientsItr_t itr = clientsThatNeedsThisObject_.find(objectId);
		if (itr != clientsThatNeedsThisObject_.end())
		{
			return itr->second;
		}
		list<NetAddr> empty;
		return empty;
	}

	list<NetAddr> MigrationGroup::clientsThatThisObjectNeeds(const Uuid& objectId) const
	{
		cObjIdToClientsItr_t itr = clientsThatThisObjectNeed_.find(objectId);
		if (itr != clientsThatThisObjectNeed_.end())
		{
			return itr->second;
		}
		list<NetAddr> empty;
		return empty;
	}

	list<NetAddr> MigrationGroup::clientsInGroup() const
	{
		list<NetAddr> clients;
		for (cObjIdToClientsItr_t ocItr = clientsThatThisObjectNeed_.begin(); ocItr != clientsThatThisObjectNeed_.end(); ++ocItr)
		{
			for (cNetAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				clients.push_back(*nalItr);
			}
		}

		for (cObjIdToClientsItr_t ocItr = clientsThatNeedsThisObject_.begin(); ocItr != clientsThatNeedsThisObject_.end(); ++ocItr)
		{
			for (cNetAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				clients.push_back(*nalItr);
			}
		}
		clients.sort();
		clients.unique();
		return clients;
	}

	void MigrationGroup::forwardAllMsgsAndUpdateAllNs(const NetAddr& newHost)
	{
		/* Update our ns, and forward potential rmis and events */
		for (cObjItr_t objItr = memberObjects_.begin(); objItr != memberObjects_.end(); ++objItr)
		{
			Ginnungagap::Instance()->migrationService()->updateNameServiceAndForwardRmiMessages(newHost, *objItr);
		}

		/* Update ns at all clients that needs one of the objects */
		for (cObjIdToClientsItr_t ocItr = clientsThatNeedsThisObject_.begin(); ocItr != clientsThatNeedsThisObject_.end(); ++ocItr)
		{
			for (cNetAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				Ginnungagap::Instance()->sendXdrBuffer(*nalItr, Ginnungagap::Instance()->nameService()->getNsRecordUpdate(ocItr->first));
			}
		}

	}

	void MigrationGroup::print() const
	{
		cerr << "******************************************" << endl;
		cerr << "Group id is: " << groupId_.getHexString() << endl;
		cerr << "active: " << active_ << endl;
		cerr << "This group contains the folwouing objects: " << endl;
		for (cObjItr_t objItr = memberObjects_.begin(); objItr != memberObjects_.end(); ++objItr)
		{
			cerr << objItr->getHexString() << endl;
		}
		for (cObjIdToClientsItr_t ocItr = clientsThatThisObjectNeed_.begin(); ocItr != clientsThatThisObjectNeed_.end(); ++ocItr)
		{
			cerr << "object id " << ocItr->first.getHexString() << " is registed to need the folowing clients" << endl;
			for (cNetAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				cerr << nalItr->addressString() << endl;
			}
		}

		for (cObjIdToClientsItr_t ocItr = clientsThatNeedsThisObject_.begin(); ocItr != clientsThatNeedsThisObject_.end(); ++ocItr)
		{
			cerr << "object id " << ocItr->first.getHexString() << " have registed that the folowing clients need this obj" << endl;
			for (cNetAddrListItr_t nalItr = ocItr->second.begin(); nalItr != ocItr->second.end(); ++nalItr)
			{
				cerr << nalItr->addressString() << endl;
			}
		}
		cerr << "++++++++++++++++++++++++++++++++++++++++++" << endl;
	}
}

