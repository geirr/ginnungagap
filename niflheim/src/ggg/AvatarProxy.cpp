/************************************************************************
*                                                                       *
*  Copyright (C) 2009 by Geir Erikstad                                  *
*  geirr@baldr.no                                                       *
*                                                                       *
*  This file is part of Niflheim.                                       *
*                                                                       *
*  Niflheim is free software: you can redistribute it and/or modify     *
*  it under the terms of the GNU General Public License as published by *
*  the Free Software Foundation, either version 2 of the License, or    *
*  (at your option) any later version.                                  *
*                                                                       *
*  Niflheim is distributed in the hope that it will be useful,          *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
*  GNU General Public License for more details.                         *
*                                                                       *
*  You should have received a copy of the GNU General Public License    *
*  along with Niflheim. If not, see <http://www.gnu.org/licenses/>.     *
*                                                                       *
************************************************************************/

#include "AvatarProxy.h"
#include "XdrSendBuffer.h"
#include "XdrReceiveBuffer.h"
#include "ObjectName.h"
#include "MessageType.h"
#include "NameService.h"
#include "Ginnungagap.h"

#include <iostream>
using std::endl;
using std::cerr;
using std::vector;
using std::pair;

using niflheim::Direction;
using niflheim::WorldObjectInfo;
using niflheim::AvatarsView;

namespace ggg
{
	typedef vector< WorldObjectInfo >::const_iterator cObjItr_t;

	AvatarProxy::AvatarProxy(const Uuid& objectId)
	{
		objectType_ = NIFLHEIM_AVATAR_OBJ;
		this->setObjectId(objectId);
		object_ = this;
		sendNeed();
	}

	AvatarProxy::~AvatarProxy()
	{
		sendDontNeed();
	}

	/* RMI */
	void AvatarProxy::move(const Direction& direction)
	{
		XdrSendBuffer* xdr = makeRmiMsg(0, INT);

		int dir = direction;

		*xdr << dir;

		sendAndGetRetVal(xdr);
	}

	void AvatarProxy::deleteAvatar()
	{
		XdrSendBuffer* xdr = makeRmiMsg(1);

		sendAndGetRetVal(xdr);
	}

	/* Events */
	void AvatarProxy::updateView(const AvatarsView& avatarsView)
	{
		XdrSendBuffer* xdr = makeEventMsg(0, INT*5 + (INT*3)*avatarsView.worldObjects.size());

		pair<int, int> vtlc = avatarsView.visibleTopLeftCorner;
		pair<int, int> vlrc = avatarsView.visibleLowerRightCorner;
		int size = avatarsView.worldObjects.size();
		
		*xdr << vtlc.first << vtlc.second << vlrc.first << vlrc.second << size;
		int wot, x, y;
		for (cObjItr_t wo = avatarsView.worldObjects.begin(); wo != avatarsView.worldObjects.end(); ++wo)
		{
			x = wo->position.first;
			y = wo->position.second;
			*xdr << x << y;
			wot = wo->worldObjectType;
			*xdr << wot;
		}

		sendEvent(xdr);
	}

	void AvatarProxy::changeWorld(const dist_ptr<niflheim::World>& newWorld)
	{
		XdrSendBuffer* xdr = makeEventMsg(1, OBJID + NETADDR);

		Uuid worldId = newWorld.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		*xdr << worldId << worldAddr;

		sendEvent(xdr);
	}

	void AvatarProxy::deactivate()
	{
		XdrSendBuffer* xdr = makeEventMsg(2);

		sendEvent(xdr);
	}

	void AvatarProxy::activate()
	{
		XdrSendBuffer* xdr = makeEventMsg(3);

		sendEvent(xdr);
	}
}

