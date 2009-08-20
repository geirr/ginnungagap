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

#include "NameServiceRecord.h"

#include "Object.h"
#include "NetAddr.h"
#include "Uuid.h"
#include "Ginnungagap.h"

namespace ggg
{
	NameServiceRecord::NameServiceRecord(const Uuid& objectId, Object* object) :
		objectId_(objectId), netAddr_(Ginnungagap::Instance()->netAddr()), local_(true), proxyCount_(0), localObjectOrProxy_(object) {}

	NameServiceRecord::NameServiceRecord(const Uuid& objectId, const NetAddr& remoteAddr) :
		objectId_(objectId), netAddr_(remoteAddr), local_(false), proxyCount_(0), localObjectOrProxy_(0) {}

	bool NameServiceRecord::isLocal() const
	{
		return local_;
	}

	Uuid NameServiceRecord::objectId() const
	{
		return objectId_;
	}

	void NameServiceRecord::setLocalObjectOrProxy(Object* localObjectOrProxy)
	{
		localObjectOrProxy_ = localObjectOrProxy;
	}

	Object* NameServiceRecord::localObjectOrProxy() const
	{
		return localObjectOrProxy_;
	}

	void NameServiceRecord::updateNetAddr(const NetAddr& netAddr)
	{
		netAddr_ = netAddr;
	}

	NetAddr NameServiceRecord::netAddr() const
	{
		return netAddr_;
	}

	void NameServiceRecord::increaseProxyCount()
	{
		if (!isLocal())
			++proxyCount_;
	}

	void NameServiceRecord::decreaseProxyCount()
	{
		if (!isLocal() && --proxyCount_ <= 0 && localObjectOrProxy_ != 0)
		{
			delete localObjectOrProxy_;
			localObjectOrProxy_ = 0;
		}
	}

	void NameServiceRecord::setToLocal(Object* object)
	{
		/* delete proxy if it exists */
		if (localObjectOrProxy_ != 0)
		{
			delete localObjectOrProxy_;
		}
		
		localObjectOrProxy_ = object;
		netAddr_ = Ginnungagap::Instance()->netAddr();
		local_ = true;
	}

	void NameServiceRecord::setToRemote(const NetAddr& remoteAddr)
	{
		netAddr_ = remoteAddr;
		/* delete object if it exists */
		if (localObjectOrProxy_ != 0)
		{
			delete localObjectOrProxy_;
		}
		localObjectOrProxy_ = 0;
		local_ = false;
	}
}

