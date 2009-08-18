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

#include "Skeleton.h"

#include "Ginnungagap.h"
#include "NameService.h"
#include "MessageType.h"
#include "ObjectName.h"
#include "XdrSendBuffer.h"

#include "TestObject.h"
#include "World.h"
#include "Avatar.h"
#include "View.h"

#include "dist_ptr.h"

#include <iostream>
using std::endl; using std::cerr;

using std::pair; using std::make_pair; using std::vector;

using niflheim::WorldObjectType; using niflheim::View; using niflheim::Direction; using niflheim::AvatarsView;
using niflheim::Avatar; using niflheim::World; using niflheim::WorldObjectInfo;

namespace ginnungagap
{
	XdrSendBuffer* Skeleton::makeRmiRetMsg(const int& size)
	{
		if (piggybackNetAddr_)
		{
			XdrSendBuffer* xdr = new XdrSendBuffer(INT*2 + OBJID + NETADDR + size);
			int ackType = RMIOKWNEWADDR;
			*xdr << rmiRetMsg_ << objectId_ << ackType << Ginnungagap::Instance()->netAddr();
			piggybackNetAddr_ = false;
			return xdr;
		}
		else
		{
			XdrSendBuffer* xdr = new XdrSendBuffer(INT*2 + OBJID + size);
			*xdr << rmiRetMsg_ << objectId_ << ackType_;
			return xdr;
		}
	}

	XdrSendBuffer* Skeleton::handleRmi(const int& objectName, const Uuid& objectId, Object* object, XdrReceiveBuffer* xdrReciveBuffer, const bool& piggybackNetAddr)
	{
		*xdrReciveBuffer >> funcNr_;
		XdrSendBuffer* xdrRet = 0;
		objectId_ = objectId;
		piggybackNetAddr_ = piggybackNetAddr;

		if (objectName == TESTOBJECT_OBJ)
		{
			TestObject* testRmi = dynamic_cast<TestObject*>(object);

			if (funcNr_ == 0)
			{
				testRmi->void_void();
				xdrRet = makeRmiRetMsg();
			}
			else if (funcNr_ == 1)
			{
				int one;
				*xdrReciveBuffer >> one;
				testRmi->void_int(one);
				xdrRet = makeRmiRetMsg();
			}
			else if (funcNr_ == 2)
			{
				int ret;
				ret = testRmi->int_void();
				xdrRet = makeRmiRetMsg(INT);
				*xdrRet << ret;
			}
			else if (funcNr_ == 3)
			{
				int one;
				*xdrReciveBuffer >> one;
				int ret;
				ret = testRmi->int_int(one);
				xdrRet = makeRmiRetMsg(INT);
				*xdrRet << ret;
			}
			else if (funcNr_ == 4)
			{
				int one, two;
				*xdrReciveBuffer >> one >> two;
				int ret;
				ret = testRmi->int_intInt(one, two);
				xdrRet = makeRmiRetMsg(INT);
				*xdrRet << ret;
			}
			else if (funcNr_ == 5)
			{
				int one, two, three;
				*xdrReciveBuffer >> one >> two >> three;
				int ret;
				ret = testRmi->int_intIntInt(one, two, three);
				xdrRet = makeRmiRetMsg(INT);
				*xdrRet << ret;
			}
			else if (funcNr_ == 6)
			{
				int numberOfInts;
				*xdrReciveBuffer >> numberOfInts;
				std::vector<int> ints(numberOfInts);
				int tmp;
				for (int i = 0; i != numberOfInts; ++i)
				{
					*xdrReciveBuffer >> tmp;
					ints[i] = tmp;
				}
				int ret;
				ret = testRmi->int_vectorOfInts(ints);
				xdrRet = makeRmiRetMsg(INT);
				*xdrRet << ret;
			}
			else
			{
				cerr << "Skeleton: Error calling right rmi for TestObject" << endl;
			}
			delete xdrReciveBuffer;
			return xdrRet;
		}
		else if (objectName == NIFLHEIM_AVATAR_OBJ)
		{
			Avatar* avatar = dynamic_cast<Avatar*>(object);

			if (funcNr_ == 0)
			{
				int direction;
				*xdrReciveBuffer >> direction;

				avatar->move((Direction) direction);
				xdrRet = makeRmiRetMsg();
			}
			else if (funcNr_ == 1)
			{
				avatar->deleteAvatar();
				xdrRet = makeRmiRetMsg();
			}
			else
			{
				cerr << "Skeleton: Error calling right rmi for Avatar" << endl;
			}
			delete xdrReciveBuffer;
			return xdrRet;
		}
		else if (objectName == NIFLHEIM_WORLD_OBJ)
		{
			World* world = dynamic_cast<World*>(object);

			if (funcNr_ == 0)
			{
				Uuid viewId;
				NetAddr viewAddr;
				*xdrReciveBuffer >> viewId >> viewAddr;

				Ginnungagap::Instance()->nameService()->updateNetAddr(viewId, viewAddr);

				dist_ptr<View> view(viewId);

				dist_ptr<Avatar> avatar = world->getNewAvatar(view);

				Uuid avatarId = avatar.objectId();
				NetAddr avatarAddr = Ginnungagap::Instance()->nameService()->netAddr(avatarId);

				xdrRet = makeRmiRetMsg(OBJID + NETADDR);

				*xdrRet << avatarId << avatarAddr;
			}
			else if (funcNr_ == 1)
			{
				pair<int, int> ret = world->getSize();
				int one = ret.first;
				int two = ret.second;
				xdrRet = makeRmiRetMsg(INT*2);
				*xdrRet << one << two;
			}
			else if (funcNr_ == 2)
			{
				int ret = world->getViewSize();
				xdrRet = makeRmiRetMsg(INT);
				*xdrRet << ret;
			}
			else
			{
				cerr << "Skeleton: Error calling right rmi for World" << endl;
			}
			delete xdrReciveBuffer;
			return xdrRet;
		}
		else
		{
			delete xdrReciveBuffer;
			cerr << "Skeleton: Unknown objectName" << endl;
			return 0;
		}
	}

	void Skeleton::handleEvent(const int& objectName, const Uuid& objectId, Object* object, XdrReceiveBuffer* xdrReciveBuffer)
	{
		*xdrReciveBuffer >> funcNr_;

		if (objectName == NIFLHEIM_WORLD_OBJ)
		{
			World* world = dynamic_cast<World*>(object);

			if (funcNr_ == 0)
			{
				Uuid avatarId;
				NetAddr avatarAddr;
				int direction;

				*xdrReciveBuffer >> avatarId >> avatarAddr >> direction;

				if (!Ginnungagap::Instance()->nameService()->exists(avatarId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(avatarId, avatarAddr);
				
				delete xdrReciveBuffer;
				world->moveAvatar(dist_ptr<Avatar>(avatarId), (Direction) direction);
				return;
			}
			else if (funcNr_ == 1)
			{
				Uuid worldId;
				NetAddr worldAddr;
				int one, two;

				*xdrReciveBuffer >> worldId >> worldAddr >> one >> two;

				if (!Ginnungagap::Instance()->nameService()->exists(worldId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
				
				delete xdrReciveBuffer;
				world->updateWithGoldmine(dist_ptr<World>(worldId), make_pair(one, two));
				return;
			}
			else if (funcNr_ == 2)
			{
				Uuid worldId;
				NetAddr worldAddr;
				int one, two;

				*xdrReciveBuffer >> worldId >> worldAddr >> one >> two;

				if (!Ginnungagap::Instance()->nameService()->exists(worldId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
				
				delete xdrReciveBuffer;
				world->updateWithAvatar(dist_ptr<World>(worldId), make_pair(one, two));
				return;
			}
			else if (funcNr_ == 3)
			{
				Uuid worldId;
				NetAddr worldAddr;
				int one, two;

				*xdrReciveBuffer >> worldId >> worldAddr >> one >> two;

				if (!Ginnungagap::Instance()->nameService()->exists(worldId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
				
				delete xdrReciveBuffer;
				world->removeBufferAvatar(dist_ptr<World>(worldId), make_pair(one, two));
				return;
			}
			else if (funcNr_ == 4)
			{
				Uuid worldId;
				NetAddr worldAddr;
				int one, two;
				int dir;

				*xdrReciveBuffer >> worldId >> worldAddr >> one >> two >> dir;

				if (!Ginnungagap::Instance()->nameService()->exists(worldId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
				
				delete xdrReciveBuffer;
				world->moveBufferAvatar(dist_ptr<World>(worldId), make_pair(one, two), (Direction) dir);
				return;
			}
			else if (funcNr_ == 5)
			{
				Uuid worldId;
				NetAddr worldAddr;
				Uuid avatarId;
				NetAddr avatarAddr;
				int one, two, three, four;

				*xdrReciveBuffer >> avatarId >> avatarAddr >> worldId >> worldAddr >> one >> two >> three >> four;

				if (!Ginnungagap::Instance()->nameService()->exists(avatarId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(avatarId, avatarAddr);

				if (!Ginnungagap::Instance()->nameService()->exists(worldId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
				
				delete xdrReciveBuffer;
				world->moveFromOtherWorld(dist_ptr<Avatar>(avatarId), dist_ptr<World>(worldId), make_pair(one, two), make_pair(three, four));
				return;
			}
			else if (funcNr_ == 6)
			{
				int one, two, three, four;

				*xdrReciveBuffer >> one >> two >> three >> four;

				delete xdrReciveBuffer;
				world->moveOK(make_pair(one, two), make_pair(three, four));
				return;
			}
			else if (funcNr_ == 7)
			{
				int one, two;

				*xdrReciveBuffer >> one >> two;

				delete xdrReciveBuffer;
				world->moveNotOK(make_pair(one, two));
				return;
			}
			else if (funcNr_ == 8)
			{
				Uuid avatarId;

				*xdrReciveBuffer >> avatarId;

				dist_ptr<Avatar> avatar(avatarId);

				world->deleteAvatar(avatar);
				return;
			}
			else
			{
				cerr << "Skeleton: Error calling right event for avatar" << endl;
			}
		}
		else if (objectName == NIFLHEIM_AVATAR_OBJ)
		{
			Avatar* avatar = dynamic_cast<Avatar*>(object);

			if (funcNr_ == 0)
			{
				AvatarsView av;
				*xdrReciveBuffer >> av.visibleTopLeftCorner.first >> av.visibleTopLeftCorner.second;
				*xdrReciveBuffer >> av.visibleLowerRightCorner.first >> av.visibleLowerRightCorner.second;
				int size;
				*xdrReciveBuffer >> size;
				int wot;
				for (int i = 0; i != size; ++i)
				{
					WorldObjectInfo woi;
					*xdrReciveBuffer >> woi.position.first >> woi.position.second;
					*xdrReciveBuffer >> wot;
					woi.worldObjectType = (WorldObjectType) wot;
					av.worldObjects.push_back(woi);
				}
				delete xdrReciveBuffer;
				avatar->updateView(av);
				return;
			}
			else if (funcNr_ == 1)
			{
				Uuid worldId;
				NetAddr worldAddr;

				*xdrReciveBuffer >> worldId >> worldAddr;

				if (!Ginnungagap::Instance()->nameService()->exists(worldId))
					Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
				
				delete xdrReciveBuffer;
				avatar->changeWorld(dist_ptr<World>(worldId));
				return;
			}
			else if (funcNr_ == 2)
			{
				delete xdrReciveBuffer;
				avatar->deactivate();
				return;
			}
			else if (funcNr_ == 3)
			{
				delete xdrReciveBuffer;
				avatar->activate();
				return;
			}
			else
			{
				cerr << "Skeleton: Error calling right event for avatar" << endl;
			}
		}
		else if (objectName == NIFLHEIM_VIEW_OBJ)
		{
			View* view = dynamic_cast<View*>(object);

			if (funcNr_ == 0)
			{
				AvatarsView av;
				*xdrReciveBuffer >> av.visibleTopLeftCorner.first >> av.visibleTopLeftCorner.second;
				*xdrReciveBuffer >> av.visibleLowerRightCorner.first >> av.visibleLowerRightCorner.second;
				int size;
				*xdrReciveBuffer >> size;
				int wot;
				for (int i = 0; i != size; ++i)
				{
					WorldObjectInfo woi;
					*xdrReciveBuffer >> woi.position.first >> woi.position.second;
					*xdrReciveBuffer >> wot;
					woi.worldObjectType = (WorldObjectType) wot;
					av.worldObjects.push_back(woi);
				}
				delete xdrReciveBuffer;
				view->updateView(av);
				return;
			}
			else
			{
				cerr << "Skeleton: Error calling right event for View" << endl;
			}
		}
		else
		{
			delete xdrReciveBuffer;
			cerr << "Skeleton: Unknown objectName" << endl;
		}
	}
}

