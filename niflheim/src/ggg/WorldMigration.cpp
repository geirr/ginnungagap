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

#include "World.h"
#include "Uuid.h"
#include "MessageType.h"
#include "ObjectName.h"
#include "XdrSendBuffer.h"

#include <iostream>

using namespace ggg;

using std::vector; using std::pair; using std::make_pair; using std::list; using std::map;

using std::cerr; using std::endl; using std::cout;

namespace niflheim
{
	struct WO
	{
		pair<int, int> pos;
		Uuid objId;
	};

	XdrSendBuffer* World::deflate()
	{
		int msg = MIGOBJ;
		int objectType = NIFLHEIM_WORLD_OBJ;

		pair<int, int> worldSize = getSize();
		int worldX = worldSize.first;
		int worldY = worldSize.second;

		list<WO> avatars;
		list<WO> goldmines;
		list< pair<int, int> > voids;
		list<WO> buffer_avatars;
		list<WO> buffer_goldmines;
		list<WO> buffer_voids;
		list<WO> buffer_nothings;
		list<WO> saveds;
		list<WO> offsets;

		list< pair<Uuid, NetAddr> > nsUpdates;

		WO wo;

		for (unsigned int x = 0; x != world_.size(); ++x)
		{
			for (unsigned int y = 0; y != world_[0].size(); ++y)
			{
				if (world_[x][y].worldObjectType == AVATAR)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].avatar->objectId();
					avatars.push_back(wo);
					nsUpdates.push_back(make_pair(world_[x][y].avatar->objectId(), Ginnungagap::Instance()->nameService()->netAddr(world_[x][y].avatar->objectId())));
				}
				else if (world_[x][y].worldObjectType == GOLDMINE)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].goldmine->objectId();
					goldmines.push_back(wo);
					nsUpdates.push_back(make_pair(world_[x][y].goldmine->objectId(), Ginnungagap::Instance()->nameService()->netAddr(world_[x][y].goldmine->objectId())));
				}
				else if (world_[x][y].worldObjectType == VOID)
				{
					voids.push_back(make_pair(x, y));
				}
				else if (world_[x][y].worldObjectType == BUFFER_AVATAR)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].world->objectId();
					buffer_avatars.push_back(wo);
					nsUpdates.push_back(make_pair(world_[x][y].world->objectId(), Ginnungagap::Instance()->nameService()->netAddr(world_[x][y].world->objectId())));
				}
				else if (world_[x][y].worldObjectType == BUFFER_GOLDMINE)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].world->objectId();
					buffer_goldmines.push_back(wo);
					nsUpdates.push_back(make_pair(world_[x][y].world->objectId(), Ginnungagap::Instance()->nameService()->netAddr(world_[x][y].world->objectId())));
				}
				else if (world_[x][y].worldObjectType == BUFFER_VOID)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].world->objectId();
					buffer_voids.push_back(wo);
				}
				else if (world_[x][y].worldObjectType == BUFFER_NOTHING)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].world->objectId();
					buffer_nothings.push_back(wo);
					nsUpdates.push_back(make_pair(world_[x][y].world->objectId(), Ginnungagap::Instance()->nameService()->netAddr(world_[x][y].world->objectId())));
				}
				else if (world_[x][y].worldObjectType == SAVED)
				{
					wo.pos = make_pair(x, y);
					wo.objId = world_[x][y].avatar->objectId();
					saveds.push_back(wo);
					nsUpdates.push_back(make_pair(world_[x][y].avatar->objectId(), Ginnungagap::Instance()->nameService()->netAddr(world_[x][y].avatar->objectId())));
				}
			}
		}

		nsUpdates.sort();
		nsUpdates.unique();

		for (map< dist_ptr<World>, pair<int, int> >::const_iterator itr = worldOffsets_.begin(); itr != worldOffsets_.end(); ++itr)
		{
			wo.objId = itr->first.objectId();
			wo.pos = itr->second;
			offsets.push_back(wo);
		}

		/* number of ints and objids in the lists: */
		int numbOfInts = 10 + 2*(avatars.size() + goldmines.size() + voids.size() + buffer_avatars.size() + buffer_goldmines.size() + buffer_voids.size() + buffer_nothings.size() + saveds.size() + offsets.size());
		int numbOfObjIds = avatars.size() + goldmines.size() + voids.size() + buffer_avatars.size() + buffer_goldmines.size() + buffer_voids.size() + buffer_nothings.size() + saveds.size() + offsets.size() + nsUpdates.size();
		int numbOfNetAddrs = nsUpdates.size();

		XdrSendBuffer* xdr = new XdrSendBuffer(INT*5 + OBJID + UUID + numbOfInts*INT + numbOfObjIds*UUID + numbOfNetAddrs*NETADDR);

		int tmp = viewSize_;

		*xdr << msg << objectType << this->objectId() << worldX << worldY << tmp << migrationGroupId_;

		/* go thorugh each list */

		int tmp2;
		Uuid tmpUuid;

		NetAddr tmpNetAddr;
		tmp = nsUpdates.size();
		*xdr << tmp;
		for (list< pair<Uuid, NetAddr> >::const_iterator itr = nsUpdates.begin(); itr != nsUpdates.end(); ++itr)
		{
			tmpUuid = itr->first;
			tmpNetAddr = itr->second;
			*xdr << tmpUuid << tmpNetAddr;
		}

		tmp = avatars.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = avatars.begin(); itr != avatars.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = goldmines.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = goldmines.begin(); itr != goldmines.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = voids.size();
		*xdr << tmp;
		for (list< pair<int, int> >::const_iterator itr = voids.begin(); itr != voids.end(); ++itr)
		{
			tmp = itr->first;
			tmp2 = itr->second;
			*xdr << tmp << tmp2;
		}

		tmp = buffer_avatars.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = buffer_avatars.begin(); itr != buffer_avatars.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = buffer_goldmines.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = buffer_goldmines.begin(); itr != buffer_goldmines.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = buffer_voids.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = buffer_voids.begin(); itr != buffer_voids.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = buffer_nothings.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = buffer_nothings.begin(); itr != buffer_nothings.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = saveds.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = saveds.begin(); itr != saveds.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		tmp = offsets.size();
		*xdr << tmp;
		for (list<WO>::const_iterator itr = offsets.begin(); itr != offsets.end(); ++itr)
		{
			tmpUuid = itr->objId;
			tmp = itr->pos.first;
			tmp2 = itr->pos.second;
			*xdr << tmpUuid << tmp << tmp2;
		}

		/* Remove from screen */
		for (unsigned int i = 0; i != world_.size(); ++i)
		{
			cout << endl;
		}
		return xdr;
	}

	World::World(XdrReceiveBuffer* xdr) : world_(0, vector< WorldObject >(0))
	{
		std::srand((unsigned)time(0));

		Uuid objId;
		int worldX, worldY, tmp;
		*xdr >> objId >> worldX >> worldY >> tmp >> migrationGroupId_;
		viewSize_ = tmp;
		this->setObjectId(objId);
		world_ = vector< vector< WorldObject > >(worldX, std::vector<WorldObject>(worldY));
		WorldObject wo;
		wo.worldObjectType = NOTHING;
		for (unsigned int x = 0; x != world_.size(); ++x)
		{
			for (unsigned int y = 0; y != world_[0].size(); ++y)
			{
				world_[x][y] = wo;
			}
		}

		int x, y;
		Uuid tmpUuid;

		NetAddr tmpNetAddr;
		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> tmpNetAddr;
			if (!(Ginnungagap::Instance()->nameService()->exists(tmpUuid)))
			{
				Ginnungagap::Instance()->nameService()->updateNetAddr(tmpUuid, tmpNetAddr);
			}
		}


		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = AVATAR;
			world_[x][y].avatar = dist_ptr<Avatar>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = GOLDMINE;
			world_[x][y].goldmine = dist_ptr<Goldmine>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> x >> y;
			world_[x][y].worldObjectType = VOID;
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = BUFFER_AVATAR;
			world_[x][y].world = dist_ptr<World>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = BUFFER_GOLDMINE;
			world_[x][y].world = dist_ptr<World>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = BUFFER_VOID;
			world_[x][y].world = dist_ptr<World>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = BUFFER_NOTHING;
			world_[x][y].world = dist_ptr<World>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			world_[x][y].worldObjectType = SAVED;
			world_[x][y].avatar = dist_ptr<Avatar>(tmpUuid);
		}

		*xdr >> tmp;
		for (int i = 0; i != tmp; ++i)
		{
			*xdr >> tmpUuid >> x >> y;
			worldOffsets_[tmpUuid] = make_pair(x, y);
		}
		drawWorld();
	}
}

