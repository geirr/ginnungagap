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

#include "NameService.h"

#include "NameServiceRecord.h"
#include "NetAddr.h"
#include "Uuid.h"
#include "Object.h"
#include "MessageType.h"
#include "XdrSendBuffer.h"
#include "XdrReceiveBuffer.h"
#include "Ginnungagap.h"

#include <map>
#include <limits>

#include <iostream>
using std::cerr;
using std::endl;

namespace ginnungagap
{
	typedef std::map<Uuid, NameServiceRecord*>::iterator regItr_t;
	typedef std::map<Uuid, NameServiceRecord*>::const_iterator cRegItr_t;

	bool NameService::exists(const Uuid& objectId) const
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr == nameServiceRegister_.end())
			return false;
		return true;
	}

	bool NameService::isLocal(const Uuid& objectId) const
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr == nameServiceRegister_.end())
			return false;
		return regItr->second->isLocal();
	}

	Object* NameService::getLocalObjectOrProxy(const Uuid& objectId) const
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr == nameServiceRegister_.end())
			return 0;
		return regItr->second->localObjectOrProxy();
	}

	void NameService::setLocalObjectOrProxy(Object* objectOrProxy)
	{
		regItr_t regItr = nameServiceRegister_.find(objectOrProxy->objectId());
		regItr->second->setLocalObjectOrProxy(objectOrProxy);
	}

	NetAddr NameService::netAddr(const Uuid& objectId) const
	{
		if (isLocal(objectId))
		{
			return Ginnungagap::Instance()->netAddr();
		}
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr == nameServiceRegister_.end())
			return NetAddr("0.0.0.0:0");
		return regItr->second->netAddr();
	}

	void NameService::bind(Object& object)
	{
		if (!(object.hasObjectId()))
		{
			object.generateObjectId();
		}
		else if (exists(object.objectId()))
		{
			Object* obj = getLocalObjectOrProxy(object.objectId());
			if (obj != 0 && obj != &object)
				delete obj;
			deleteNameServiceRecord(object.objectId());
		}
		NameServiceRecord* nsr = new NameServiceRecord(object.objectId(), &object);
		nameServiceRegister_[object.objectId()] = nsr;
	}

	void NameService::setAsRemote(const Uuid& objectId, const NetAddr& netAddr)
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr == nameServiceRegister_.end())
		{
			cerr << "Error: Object is not in NameService, can't set to remote" << endl;
			exit(1);
		}
		regItr->second->setToRemote(netAddr);
	}

	void NameService::addRemoteObject(const Uuid& objectId, const NetAddr& netAddr)
	{
		NameServiceRecord* nsr = new NameServiceRecord(objectId, netAddr);
		nameServiceRegister_[objectId] = nsr;
	}

	void NameService::updateNetAddr(const Uuid& objectId, const NetAddr& netAddr)
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr == nameServiceRegister_.end())
		{
			addRemoteObject(objectId, netAddr);
		}
		else
		{
			regItr->second->updateNetAddr(netAddr);
		}
	}

	void NameService::deleteNameServiceRecord(const Uuid& objectId)
	{
		regItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr != nameServiceRegister_.end())
		{
			NameServiceRecord* nsr = regItr->second;
			nameServiceRegister_.erase(regItr);
			delete nsr;
		}

	}

	XdrSendBuffer* NameService::getNsRecordUpdate(const Uuid& objectId) const
	{
		if (!exists(objectId))
		{
			cerr << "Error: ObjectId doesn't exist in NameService" << endl;
			exit(1);
		}
		NetAddr netAddr = this->netAddr(objectId);
		XdrSendBuffer* xdr = new XdrSendBuffer(INT+OBJID+NETADDR);
		int msg = NSUPDATE;
		*xdr << msg << objectId << netAddr;
		return xdr;
	}

	void NameService::updateNsRecord(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid objectId;
		NetAddr netAddr;
		*xdrBuffer >> objectId >> netAddr;
		if (!isLocal(objectId))
		{
			updateNetAddr(objectId, netAddr);	
		}
		delete xdrBuffer;
	}

	void NameService::increaseProxyCount(const Uuid& objectId)
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr != nameServiceRegister_.end())
		{
			regItr->second->increaseProxyCount();
		}
	}

	void NameService::decreaseProxyCount(const Uuid& objectId)
	{
		cRegItr_t regItr = nameServiceRegister_.find(objectId);
		if (regItr != nameServiceRegister_.end())
		{
			regItr->second->decreaseProxyCount();
		}
	}
}

