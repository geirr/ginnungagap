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

#include "WorldProxy.h"
#include "XdrSendBuffer.h"
#include "XdrReceiveBuffer.h"
#include "NameService.h"
#include "Ginnungagap.h"
#include "ObjectName.h"

#include <iostream>

using std::endl;
using std::cerr;

using std::pair;
using std::make_pair;

using niflheim::Avatar;
using niflheim::View;
using niflheim::World;
using niflheim::Direction;

namespace ggg
{
	WorldProxy::WorldProxy(const Uuid& objectId)
	{
		objectType_ = NIFLHEIM_WORLD_OBJ;
		object_ = this;
		this->setObjectId(objectId);
		sendNeed();
	}

	WorldProxy::~WorldProxy()
	{
		sendDontNeed();
	}

	/* RMI */
	dist_ptr<Avatar> WorldProxy::getNewAvatar(const dist_ptr<View>& view)
	{

		XdrSendBuffer* xdr = makeRmiMsg(0, OBJID + NETADDR);

		Uuid viewId = view.objectId();
		NetAddr viewAddr = Ginnungagap::Instance()->nameService()->netAddr(viewId);

		*xdr << viewId << viewAddr;

		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);

		Uuid avatarId;
		NetAddr avatarAddr;

		*xdrReciveBuffer >> avatarId >> avatarAddr;
		delete xdrReciveBuffer;

		Ginnungagap::Instance()->nameService()->updateNetAddr(avatarId, avatarAddr);

		return dist_ptr<Avatar>(avatarId);
	}


	pair<int, int> WorldProxy::getSize()
	{
		XdrSendBuffer* xdr = makeRmiMsg(1);

		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);

		int one, two;
		*xdrReciveBuffer >> one >> two;
		delete xdrReciveBuffer;

		return make_pair(one, two);
	}

	int WorldProxy::getViewSize()
	{
		XdrSendBuffer* xdr = makeRmiMsg(2);

		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);

		int size;
		*xdrReciveBuffer >> size;
		delete xdrReciveBuffer;

		return size;

	}

	/* EVENTS */
	void WorldProxy::moveAvatar(dist_ptr<Avatar> avatar, const Direction& direction)
	{
		XdrSendBuffer* xdr = makeEventMsg(0, OBJID + NETADDR + INT);

		Uuid avatarId = avatar.objectId();
		NetAddr avatarAddr = Ginnungagap::Instance()->nameService()->netAddr(avatarId);
		int direc = direction;

		*xdr << avatarId << avatarAddr << direc;

		sendEvent(xdr);
	}

	void WorldProxy::updateWithGoldmine(const dist_ptr<World>& world, const pair<int, int>& position)
	{
		XdrSendBuffer* xdr = makeEventMsg(1, OBJID + NETADDR + INT*2);

		Uuid worldId = world.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		int one = position.first;
		int two = position.second;

		*xdr << worldId << worldAddr << one << two;

		sendEvent(xdr);
	}

	void WorldProxy::updateWithAvatar(const dist_ptr<World>& world, const pair<int, int>& position)
	{
		XdrSendBuffer* xdr = makeEventMsg(2, OBJID + NETADDR + INT*2);

		Uuid worldId = world.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		int one = position.first;
		int two = position.second;

		*xdr << worldId << worldAddr << one << two;

		sendEvent(xdr);
	}

	void WorldProxy::removeBufferAvatar(const dist_ptr<World>& world, const pair<int, int>& position)
	{
		XdrSendBuffer* xdr = makeEventMsg(3, OBJID + NETADDR + INT*2);

		Uuid worldId = world.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		int one = position.first;
		int two = position.second;

		*xdr << worldId << worldAddr << one << two;

		sendEvent(xdr);
	}

	void WorldProxy::moveBufferAvatar(const dist_ptr<World>& world, const pair<int, int>& position, const Direction& direction)
	{
		XdrSendBuffer* xdr = makeEventMsg(4, OBJID + NETADDR + INT*3);

		Uuid worldId = world.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		int one = position.first;
		int two = position.second;

		int dir = direction;

		*xdr << worldId << worldAddr << one << two << dir;

		sendEvent(xdr);
	}

	void WorldProxy::moveFromOtherWorld(const dist_ptr<Avatar>& avatar, dist_ptr<World> world, const pair<int, int>& from, const pair<int, int>& to)
	{
		XdrSendBuffer* xdr = makeEventMsg(5, OBJID*2 + NETADDR*2 + INT*4);

		Uuid avatarId = avatar.objectId();
		NetAddr avatarAddr = Ginnungagap::Instance()->nameService()->netAddr(avatarId);

		Uuid worldId = world.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		int one = from.first;
		int two = from.second;

		int three = to.first;
		int four = to.second;

		*xdr << avatarId << avatarAddr << worldId << worldAddr << one << two << three << four;

		sendEvent(xdr);
	}

	void WorldProxy::moveOK(const pair<int, int>& from, const pair<int, int>& to)
	{
		XdrSendBuffer* xdr = makeEventMsg(6, INT*4);

		int one = from.first;
		int two = from.second;

		int three = to.first;
		int four = to.second;

		*xdr << one << two << three << four;

		sendEvent(xdr);
	}

	void WorldProxy::moveNotOK(const pair<int, int>& from)
	{
		XdrSendBuffer* xdr = makeEventMsg(7, INT*2);

		int one = from.first;
		int two = from.second;

		*xdr << one << two;

		sendEvent(xdr);
	}

	void WorldProxy::deleteAvatar(const dist_ptr<Avatar>& avatar)
	{
		XdrSendBuffer* xdr = makeEventMsg(8, OBJID);

		Uuid avatarId = avatar.objectId();

		*xdr << avatarId;

		sendEvent(xdr);
	}
}

