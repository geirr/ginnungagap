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

#include "Proxy.h"

#include "XdrSendBuffer.h"
#include "XdrReceiveBuffer.h"
#include "ObjectName.h"
#include "MessageType.h"
#include "NameService.h"
#include "Ginnungagap.h"
#include "Object.h"
#include "MigrationService.h"

#include <sstream>
#include <iostream>
using std::endl;
using std::cerr;

using std::stringstream;

namespace ginnungagap
{
	void Proxy::sendNeed() const
	{
		if (Ginnungagap::Instance()->nameService()->isLocal(object_->objectId()))
			return;
		if (Ginnungagap::Instance()->appType() == CLIENT)
		{
			XdrSendBuffer* xdr = new XdrSendBuffer(INT + NETADDR + OBJID);
			int msg = NEED;
			*xdr << msg << Ginnungagap::Instance()->netAddr() << object_->objectId();
			Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->nameService()->netAddr(object_->objectId()), xdr);
		}
	}

	void Proxy::sendDontNeed() const
	{
		if (Ginnungagap::Instance()->nameService()->isLocal(object_->objectId()))
			return;
		if (Ginnungagap::Instance()->appType() == CLIENT)
		{
			XdrSendBuffer* xdr = new XdrSendBuffer(INT + NETADDR + OBJID);
			int msg = DONTNEED;
			*xdr << msg << Ginnungagap::Instance()->netAddr() << object_->objectId();
			Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->nameService()->netAddr(object_->objectId()), xdr);
		}
	}

	XdrReceiveBuffer* Proxy::sendAndGetRetVal(XdrSendBuffer* xdrBuffer)
	{
		if (Ginnungagap::Instance()->nameService()->isLocal(object_->objectId()))
		{
			cerr << "Calling RMI on an object in migrating state. Sorry, but that's not implemented yet." << endl;
			exit(1);
		}

		int done = false;
		XdrReceiveBuffer* xdrReciveBuffer;
		while (!done)
		{
			XdrSendBuffer* copy = new XdrSendBuffer(*xdrBuffer);
			NetAddr remoteAddr = Ginnungagap::Instance()->nameService()->netAddr(object_->objectId());
			Ginnungagap::Instance()->sendXdrBuffer(remoteAddr, copy);

			xdrReciveBuffer = Ginnungagap::Instance()->getRmiRetVal(object_->objectId(), remoteAddr);

			Uuid objectId;
			*xdrReciveBuffer >> objectId;

			if (objectId != object_->objectId())
			{
				cerr << "Got wrong objectid, should not happend..." << endl;
				exit(0);
			}

			int	ackType;
			*xdrReciveBuffer >> ackType;

			if (ackType == RMIOK)
			{
				done = true;
			}
			else if (ackType == RMIOKWNEWADDR)
			{
				done = true;
				NetAddr newAddr;
				*xdrReciveBuffer >> newAddr;
				Ginnungagap::Instance()->nameService()->updateNetAddr(object_->objectId(), newAddr);
				stringstream logMsg;
				logMsg << "ProxyObject: updating nameservice for object: " << object_->objectId().getHexString() << " to address: " << newAddr.addressString();
				Ginnungagap::Instance()->writeToLog(logMsg.str());
			}
			else if (ackType == NEWADDR)
			{
				NetAddr newAddr;
				*xdrReciveBuffer >> newAddr;
				Ginnungagap::Instance()->nameService()->updateNetAddr(object_->objectId(), newAddr);
				stringstream logMsg;
				logMsg << "ProxyObject: got NEWADDR: updating nameservice for object: " << object_->objectId().getHexString() << " to address: " << newAddr.addressString();
				Ginnungagap::Instance()->writeToLog(logMsg.str());
			}
			else if (ackType == UNKNOWN)
			{
				cerr << "Object is unknown at server" << endl;
				exit(0);
			}
		}
		delete xdrBuffer;
		return xdrReciveBuffer;
	}

	void Proxy::sendEvent(XdrSendBuffer* xdrBuffer)
	{
		if (Ginnungagap::Instance()->nameService()->isLocal(object_->objectId()))
		{
			Ginnungagap::Instance()->migrationService()->putXdrBufferInQueue(object_->objectId(), xdrBuffer);
		}
		else
		{
			NetAddr remoteAddr = Ginnungagap::Instance()->nameService()->netAddr(object_->objectId());
			Ginnungagap::Instance()->sendXdrBuffer(remoteAddr, xdrBuffer);
		}
	}

	XdrSendBuffer* Proxy::makeRmiMsg(int funcNr, const int& size)
	{
		XdrSendBuffer* xdr = new XdrSendBuffer(stdSize_ + size);
		*xdr << rmi_ << objectType_ << object_->objectId() << funcNr;
		return xdr;
	}

	XdrSendBuffer* Proxy::makeEventMsg(int funcNr, const int& size)
	{
		XdrSendBuffer* xdr = new XdrSendBuffer(stdSize_ + size);
		*xdr << event_ << objectType_ << object_->objectId() << funcNr;
		return xdr;
	}
}

