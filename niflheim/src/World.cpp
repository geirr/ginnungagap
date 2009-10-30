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

#include "Ginnungagap.h"
#include "MigrationGroup.h"
#include "MigrationService.h"
#include "Uuid.h"

#include <iostream>

using std::cout; using std::cerr; using std::endl;

using std::vector; using std::list; using std::make_pair; using std::pair; using std::map;

using ggg::dist_ptr; using ggg::Ginnungagap;
using ggg::MigrationGroup; using ggg::Uuid;

namespace niflheim
{
	typedef vector< vector<WorldObject> >::const_iterator cX_Itr_t;
	typedef vector< vector<WorldObject> >::iterator x_Itr_t;
	typedef vector<WorldObject>::const_iterator cY_Itr_t;
	typedef vector<WorldObject>::iterator y_Itr_t;

	typedef list< dist_ptr<Avatar> >::iterator avatarList_Itr_t;
	typedef list< dist_ptr<Avatar> >::const_iterator cAvatarList_Itr_t;

	typedef list< dist_ptr<World> >::iterator worldList_Itr_t;
	typedef list< dist_ptr<World> >::const_iterator cWorldList_Itr_t;

	typedef map< dist_ptr<World>, pair<int, int> >::const_iterator cWorldOffsetItr_t;

	World::World(const int& xLen, const int& yLen, const int& viewSize, const Uuid& objectId, const dist_ptr<World>& worldAround) : world_(xLen, std::vector<WorldObject>(yLen)), viewSize_(viewSize)
	{
		this->setObjectId(objectId);
		std::srand((unsigned)time(0));
		if (!worldAround.isNull())
		{
			world_ = vector< vector<WorldObject> >(xLen+viewSize_-1, std::vector<WorldObject>(yLen+viewSize_-1));
		}

		/* create migration group, and add it to ggg */
		MigrationGroup* mg = new MigrationGroup();
		migrationGroupId_ = mg->groupId();
		mg->addObject(this->objectId());
		for (unsigned int x = 0; x != world_.size(); ++x)
		{
			for (unsigned int y = 0; y != world_[0].size(); ++y)
			{
				world_[x][y].worldObjectType = NOTHING;
				if (!worldAround.isNull() && (x < viewSize_/2 || x >= world_.size()-viewSize_/2 || y < viewSize_/2 || y >= world_[0].size()-viewSize_/2))
				{
					world_[x][y].worldObjectType = BUFFER_NOTHING;
					world_[x][y].world = worldAround;
				}
			}
		}

		/* Bind the world! */
		Ginnungagap::Instance()->nameService()->bind(*this);
		Ginnungagap::Instance()->migrationService()->addMigrationGroup(mg);
		drawWorld();
	}

	dist_ptr<Avatar> World::getNewAvatar(const dist_ptr<View>& view)
	{
		dist_ptr<Avatar> newAvatar;

		int xRandDiff = std::rand()%world_.size();
		int yRandDiff = std::rand()%world_[0].size();

		for (unsigned int x = 0; x != world_.size(); ++x)
		{
			for (unsigned int y = 0; y != world_[0].size(); ++y)
			{
				int xRand = (x+xRandDiff)%world_.size();
				int yRand = (y+yRandDiff)%world_[0].size();
				if (isFree(make_pair(xRand, yRand)))
				{
					newAvatar = new Avatar(dist_ptr<World>(this), view);
					Ginnungagap::Instance()->nameService()->bind(*newAvatar);
					world_[xRand][yRand].worldObjectType = AVATAR;
					world_[xRand][yRand].avatar = newAvatar;
					Ginnungagap::Instance()->migrationService()->getMigrationGroupById(migrationGroupId_)->addObject(newAvatar.objectId());
					updateAffectedAvatarsViews(make_pair(xRand, yRand));

					list< dist_ptr<World> > affectedWorlds = getWorldsInLOS(make_pair(xRand, yRand));
					for (worldList_Itr_t affWorld = affectedWorlds.begin(); affWorld != affectedWorlds.end(); ++affWorld)
					{
						(*affWorld)->updateWithAvatar(dist_ptr<World>(this), make_pair(xRand, yRand));
					}
					drawWorld();
					return newAvatar;
				}
			}
		}

		/* If the world is full, return a null pointer */
		return newAvatar;
	}

	void World::deleteAvatar(const dist_ptr<Avatar>& avatar)
	{
		pair<int, int> pos = getPosition(avatar);
		world_[pos.first][pos.second].avatar.reset();
		world_[pos.first][pos.second].worldObjectType = NOTHING;

		/* remove avatar from migration group */
		Ginnungagap::Instance()->migrationService()->getMigrationGroupById(migrationGroupId_)->removeObject(avatar.objectId());

		/* delete avatar if a local object */
		if (Ginnungagap::Instance()->nameService()->isLocal(avatar.objectId()))
		{
			Avatar* avatarPtr = dynamic_cast<Avatar*>(Ginnungagap::Instance()->nameService()->getLocalObjectOrProxy(avatar.objectId()));
			delete avatarPtr;
		}

		updateAffectedAvatarsViews(pos);
		list< dist_ptr<World> > affectedWorlds = getWorldsInLOS(pos);
		for (worldList_Itr_t affWorld = affectedWorlds.begin(); affWorld != affectedWorlds.end(); ++affWorld)
		{
			(*affWorld)->removeBufferAvatar(dist_ptr<World>(this), pos);
		}
		drawWorld();
	}

	void World::moveAvatar(dist_ptr<Avatar> avatar, const Direction& direction)
	{
		pair<int, int> position = getPosition(avatar);
		pair<int, int> newPosition;
		if (direction == NORTH)
		{
			newPosition = make_pair(position.first-1, position.second);
		}
		else if (direction == SOUTH)
		{
			newPosition = make_pair(position.first+1, position.second);
		}
		else if (direction == WEST)
		{
			newPosition = make_pair(position.first, position.second-1);
		}
		else if (direction == EAST)
		{
			newPosition = make_pair(position.first, position.second+1);
		}
		else
		{
			/* we have tried to move in direction NONE */
			return;
		}

		/* check if we try to move outside the world */
		pair<int, int> max = getSize();
		if ((newPosition.first < 0 || newPosition.first >= max.first)
				|| (newPosition.second < 0 || newPosition.second >= max.second))
		{
			return;
		}

		/* Check if the place we want to move to is free */
		if (!isFree(newPosition) && !(world_[newPosition.first][newPosition.second].worldObjectType == BUFFER_NOTHING))
		{
			return;
		}

		/* check if we need to change world */
		if (world_[newPosition.first][newPosition.second].worldObjectType == BUFFER_NOTHING)
		{
			/* try to move onto the new world */
			avatar->deactivate();
			world_[position.first][position.second].worldObjectType = SAVED;
			world_[newPosition.first][newPosition.second].world->moveFromOtherWorld(avatar, dist_ptr<World>(this), position, newPosition);
		}
		else
		{

			/* Move */
			world_[newPosition.first][newPosition.second].worldObjectType = AVATAR;
			world_[newPosition.first][newPosition.second].avatar = avatar;
			world_[position.first][position.second].worldObjectType = NOTHING;
			world_[position.first][position.second].avatar.reset();
			updateAffectedAvatarsViews(position, direction);
			drawWorld();

			/* update other worlds buffers */
			list< dist_ptr<World> > affectedWorlds = getWorldsInLOS(position, direction);
			for (worldList_Itr_t affWorld = affectedWorlds.begin(); affWorld != affectedWorlds.end(); ++affWorld)
			{
				(*affWorld)->moveBufferAvatar(dist_ptr<World>(this), position, direction);
			}
		}
	}

	void World::moveFromOtherWorld(const dist_ptr<Avatar>& avatar, dist_ptr<World> world, const pair<int, int>& from, const pair<int, int>& to)
	{
		/* check if world is free, if not, revert move */
		pair<int, int> position = getPositionWithOffset(world, to);
		pair<int, int> oldPosition = getPositionWithOffset(world, from);
		if (isFree(position))
		{
			world_[position.first][position.second].worldObjectType = AVATAR;
			world_[position.first][position.second].avatar = avatar;
			world_[oldPosition.first][oldPosition.second].worldObjectType = BUFFER_NOTHING;
			world_[oldPosition.first][oldPosition.second].avatar.reset();
			world_[oldPosition.first][oldPosition.second].world = world;
			world->moveOK(from, to);
			updateAffectedAvatarsViews(position);
			drawWorld();
		}
		else
		{
			world->moveNotOK(from);
		}
	}

	void World::moveOK(const pair<int, int>& from, const pair<int, int>& to)
	{
		world_[from.first][from.second].avatar->changeWorld(world_[to.first][to.second].world);
		world_[from.first][from.second].avatar->activate();
		Ginnungagap::Instance()->migrationService()->getMigrationGroup(world_[from.first][from.second].avatar.objectId())->
			moveObjectToGroupWithMainObject(world_[from.first][from.second].avatar.objectId(), world_[to.first][to.second].world.objectId());
		world_[from.first][from.second].worldObjectType = NOTHING;
		world_[from.first][from.second].avatar.reset();
		world_[to.first][to.second].worldObjectType = BUFFER_AVATAR;
		updateAffectedAvatarsViews(from);
		drawWorld();
	}

	void World::moveNotOK(const pair<int, int>& from)
	{	
		world_[from.first][from.second].worldObjectType = AVATAR;
		world_[from.first][from.second].avatar->activate();
	}

	pair<int, int> World::getSize()
	{
		return make_pair(world_.size(), world_[0].size());
	}

	int World::getViewSize()
	{
		return viewSize_;
	}

	void World::updateWithGoldmine(const dist_ptr<World>& world, const pair<int, int>& position)
	{
		pair<int, int> pos = getPositionWithOffset(world, position);
		world_[pos.first][pos.second].worldObjectType = BUFFER_GOLDMINE;
		world_[pos.first][pos.second].world = world;
	}

	void World::updateWithAvatar(const dist_ptr<World>& world, const pair<int, int>& position)
	{
		pair<int, int> pos = getPositionWithOffset(world, position);
		world_[pos.first][pos.second].worldObjectType = BUFFER_AVATAR;
		world_[pos.first][pos.second].world = world;
	}

	void World::removeBufferAvatar(const dist_ptr<World>& world, const pair<int, int>& position)
	{
		pair<int, int> pos = getPositionWithOffset(world, position);
		world_[pos.first][pos.second].worldObjectType = BUFFER_NOTHING;
		world_[pos.first][pos.second].world = world;
	}

	void World::moveBufferAvatar(const dist_ptr<World>& world, const pair<int, int>& position, const Direction& direction)
	{
		pair<int, int> oldPosition = getPositionWithOffset(world, position);
		pair<int, int> newPosition;
		if (direction == NORTH)
		{
			newPosition = make_pair(oldPosition.first-1, oldPosition.second);
		}
		else if (direction == SOUTH)
		{
			newPosition = make_pair(oldPosition.first+1, oldPosition.second);
		}
		else if (direction == WEST)
		{
			newPosition = make_pair(oldPosition.first, oldPosition.second-1);
		}
		else if (direction == EAST)
		{
			newPosition = make_pair(oldPosition.first, oldPosition.second+1);
		}
		else
		{
			/* we have tried to move in direction NONE */
			return;
		}

		/* Move */
		/* check if both are inside our world */
		pair<int, int> max = getSize();

		if (!(newPosition.first < 0 || newPosition.first >= max.first || newPosition.second < 0 || newPosition.second >= max.second) && world_[newPosition.first][newPosition.second].worldObjectType != BUFFER_VOID)
		{
			world_[newPosition.first][newPosition.second].worldObjectType = BUFFER_AVATAR;
			world_[newPosition.first][newPosition.second].world = world;
		}
		if (!(oldPosition.first < 0 || oldPosition.first >= max.first || oldPosition.second < 0 || oldPosition.second >= max.second) && world_[oldPosition.first][oldPosition.second].worldObjectType != BUFFER_VOID)
		{
			world_[oldPosition.first][oldPosition.second].worldObjectType = BUFFER_NOTHING;
			world_[oldPosition.first][oldPosition.second].world = world;
		}
		updateAffectedAvatarsViews(oldPosition, direction);
		drawWorld();
	}

	void World::drawWorld() const
	{
		cout << "+";
		for (unsigned int i = 0; i != world_[0].size(); ++i)
		{
			cout << "--";
		}
		cout << "-+";
		cout << endl;
		for (cX_Itr_t x_itr = world_.begin(); x_itr != world_.end(); ++x_itr)
		{
			cout << "|";
			for (cY_Itr_t y_itr = x_itr->begin(); y_itr != x_itr->end(); ++y_itr)
			{
				if (y_itr->worldObjectType == AVATAR)
				{
					cout << " A";
				}
				else if (y_itr->worldObjectType == GOLDMINE)
				{
					cout << " G";
				}
				else if (y_itr->worldObjectType == NOTHING || y_itr->worldObjectType == SAVED)
				{
					cout << " .";
				}
				else if (y_itr->worldObjectType == VOID)
				{
					cout << " *";
				}
				else if (y_itr->worldObjectType == BUFFER_AVATAR)
				{
					cout << " a";
				}
				else if (y_itr->worldObjectType == BUFFER_GOLDMINE)
				{
					cout << " g";
				}
				else if (y_itr->worldObjectType == BUFFER_NOTHING)
				{
					cout << " ,";
				}
				else if (y_itr->worldObjectType == BUFFER_VOID)
				{
					cout << "  ";
				}
				else
				{
					cout << " X";
				}
			}
			cout << " |" << endl;
		}
		cout << "+";
		for (unsigned int i = 0; i != world_[0].size(); ++i)
		{
			cout << "--";
		}
		cout << "-+";
		cout << endl;
	}

	bool World::isFree(const pair<int, int>& position) const
	{
		return (world_[position.first][position.second].worldObjectType == NOTHING);
	}

	pair<int, int> World::getPosition(const dist_ptr<Avatar>& avatar) const
	{
		for (unsigned int x = 0; x != world_.size(); ++x)
		{
			for (unsigned int y = 0; y != world_[0].size(); ++y)
			{
				if (world_[x][y].worldObjectType == AVATAR && world_[x][y].avatar == avatar)
				{
					return make_pair(x, y);
				}
			}
		}
		return make_pair(-1, -1);
	}

	pair<int, int> World::getPosition(const dist_ptr<Goldmine>& goldmine) const
	{
		for (unsigned int x = 0; x != world_.size(); ++x)
		{
			for (unsigned int y = 0; y != world_[0].size(); ++y)
			{
				if (world_[x][y].worldObjectType == GOLDMINE && world_[x][y].goldmine == goldmine)
				{
					return make_pair(x, y);
				}
			}
		}
		return make_pair(-1, -1);
	}

	void World::updateAffectedAvatarsViews(const pair<int, int>& position, const Direction& direction)
	{
		list< dist_ptr< Avatar> > affectedAvatars = getAvatarsInLOS(position, direction);
		for (avatarList_Itr_t avatar = affectedAvatars.begin(); avatar != affectedAvatars.end(); ++ avatar)
		{
			(*avatar)->updateView(getAvatarsView(*avatar));
		}
	}

	list< dist_ptr<Avatar> > World::getAvatarsInLOS(const pair<int, int>& position, const Direction& direction)
	{
		list< dist_ptr<Avatar> > avatarsInLOS;
		int half = viewSize_/2;
		pair<int, int> topLeftCorner;
		pair<int, int> lowerRightCorner;

		topLeftCorner = make_pair(position.first-half, position.second-half);
		lowerRightCorner = make_pair(position.first+half, position.second+half);

		if (direction == NORTH)
		{
			--topLeftCorner.first;
		}
		else if (direction == EAST)
		{
			++lowerRightCorner.second;
		}
		else if (direction == SOUTH)
		{
			++lowerRightCorner.first;
		}
		else if (direction == WEST)
		{
			--topLeftCorner.second;
		}

		if (topLeftCorner.first < 0)
			topLeftCorner.first = 0;
		if (topLeftCorner.second < 0)
			topLeftCorner.second = 0;
		pair <int, int> size = getSize();
		--(size.first);
		--(size.second);
		if (lowerRightCorner.first > size.first)
			lowerRightCorner.first = size.first;
		if (lowerRightCorner.second > size.second)
			lowerRightCorner.second = size.second;

		for (int x = topLeftCorner.first; x != lowerRightCorner.first+1; ++x)
		{
			for (int y = topLeftCorner.second; y != lowerRightCorner.second+1; ++y)
			{
				if (world_[x][y].worldObjectType == AVATAR)
				{
					avatarsInLOS.push_back(world_[x][y].avatar);
				}
			}
		}
		return avatarsInLOS;
	}

	list< dist_ptr<World> > World::getWorldsInLOS(const pair<int, int>& position, const Direction& direction)
	{
		list< dist_ptr<World> > worldsInLOS;
		int half = viewSize_/2;

		pair<int, int> topLeftCorner;
		pair<int, int> lowerRightCorner;

		topLeftCorner = make_pair(position.first-half, position.second-half);
		lowerRightCorner = make_pair(position.first+half, position.second+half);

		if (direction == NORTH)
		{
			--topLeftCorner.first;
		}
		else if (direction == EAST)
		{
			++lowerRightCorner.second;
		}
		else if (direction == SOUTH)
		{
			++lowerRightCorner.first;
		}
		else if (direction == WEST)
		{
			--topLeftCorner.second;
		}

		if (topLeftCorner.first < 0)
			topLeftCorner.first = 0;
		if (topLeftCorner.second < 0)
			topLeftCorner.second = 0;
		pair <int, int> size = getSize();
		--(size.first);
		--(size.second);
		if (lowerRightCorner.first > size.first)
			lowerRightCorner.first = size.first;
		if (lowerRightCorner.second > size.second)
			lowerRightCorner.second = size.second;

		for (int x = topLeftCorner.first; x != lowerRightCorner.first+1; ++x)
		{
			for (int y = topLeftCorner.second; y != lowerRightCorner.second+1; ++y)
			{
				if (world_[x][y].worldObjectType == BUFFER_AVATAR || world_[x][y].worldObjectType == BUFFER_GOLDMINE || world_[x][y].worldObjectType == BUFFER_NOTHING)
				{
					worldsInLOS.push_back(world_[x][y].world);
				}
			}
		}
		worldsInLOS.sort();
		worldsInLOS.unique();
		return worldsInLOS;
	}

	AvatarsView World::getAvatarsView(const ggg::dist_ptr<Avatar>& avatar)
	{
		int half = viewSize_/2;
		pair<int, int> pos = getPosition(avatar);
		pair<int, int> max = getSize();
		--max.first;
		--max.second;

		AvatarsView av;
		av.visibleTopLeftCorner.first = 0;
		av.visibleLowerRightCorner.first = viewSize_;
		av.visibleTopLeftCorner.second = 0;
		av.visibleLowerRightCorner.second = viewSize_;

		pair<int, int> topLeftCorner = make_pair(pos.first-half, pos.second-half);
		pair<int, int> lowerRightCorner = make_pair(pos.first+half, pos.second+half);

		if (topLeftCorner.first < 0)
		{
			av.visibleTopLeftCorner.first = - topLeftCorner.first;
			topLeftCorner.first = 0;
		}
		if (topLeftCorner.second < 0)
		{
			av.visibleTopLeftCorner.second = - topLeftCorner.second;
			topLeftCorner.second = 0;
		}
		if (lowerRightCorner.first > max.first)
		{
			av.visibleLowerRightCorner.first -= lowerRightCorner.first - max.first;
			lowerRightCorner.first = max.first;
		}
		if (lowerRightCorner.second > max.second)
		{
			av.visibleLowerRightCorner.second -= lowerRightCorner.second - max.second;
			lowerRightCorner.second = max.second;
		}

		int x = av.visibleTopLeftCorner.first;
		for (int wx = topLeftCorner.first; wx != lowerRightCorner.first+1; ++wx)
		{
			int y = av.visibleTopLeftCorner.second;
			for (int wy = topLeftCorner.second; wy != lowerRightCorner.second+1; ++wy)
			{
				if (world_[wx][wy].worldObjectType == AVATAR || world_[wx][wy].worldObjectType == BUFFER_AVATAR)
				{
					WorldObjectInfo woi;
					woi.position = make_pair(x, y);
					woi.worldObjectType = AVATAR;
					av.worldObjects.push_back(woi);
				}
				else if (world_[wx][wy].worldObjectType == GOLDMINE || world_[wx][wy].worldObjectType == BUFFER_GOLDMINE)
				{
					WorldObjectInfo woi;
					woi.position = make_pair(x, y);
					woi.worldObjectType = GOLDMINE;
					av.worldObjects.push_back(woi);
				}
				++y;
			}
			++x;
		}
		return av;
	}

	void World::addGoldmine(const std::pair<int, int>& goldmineCoords)
	{
		dist_ptr<Goldmine> goldmine = new Goldmine(dist_ptr<World>(this));
		bool bufferAround = false;
		if (world_[0][0].worldObjectType == BUFFER_NOTHING)
		{
			world_[goldmineCoords.first+viewSize_/2][goldmineCoords.second+viewSize_/2].worldObjectType = GOLDMINE;
			world_[goldmineCoords.first+viewSize_/2][goldmineCoords.second+viewSize_/2].goldmine = goldmine;
			bufferAround = true;
		}
		else
		{
			world_[goldmineCoords.first][goldmineCoords.second].worldObjectType = GOLDMINE;
			world_[goldmineCoords.first][goldmineCoords.second].goldmine = goldmine;
		}
		Ginnungagap::Instance()->migrationService()->getMigrationGroupById(migrationGroupId_)->addObject(goldmine->objectId());
		Ginnungagap::Instance()->nameService()->bind(*goldmine);

		list< dist_ptr<World> > affectedWorlds = getWorldsInLOS(goldmineCoords);
		for (worldList_Itr_t affWorld = affectedWorlds.begin(); affWorld != affectedWorlds.end(); ++affWorld)
		{
			if (bufferAround)
			{
				(*affWorld)->updateWithGoldmine(dist_ptr<World>(this), make_pair(goldmineCoords.first+viewSize_/2, goldmineCoords.second+viewSize_/2));
			}
			else
			{
				(*affWorld)->updateWithGoldmine(dist_ptr<World>(this), goldmineCoords);
			}
		}
		drawWorld();
	}

	dist_ptr<World> World::addWorldWithin(const std::pair<int, int>& topLeftCorner, const std::pair<int, int>& lowerRightCorner, const Uuid& objectId)
	{
		/* (lowerRightCorner - topLeftCorner + 1) + (viewSize_ - 1) = */
		/* (new world size) + (buffer zone) */
		int xlen = lowerRightCorner.first - topLeftCorner.first + 1;
		int ylen = lowerRightCorner.second - topLeftCorner.second + 1;

		dist_ptr<World> worldWithin = new World(xlen, ylen, viewSize_, objectId, dist_ptr<World>(this));
		
		/* change this world to BUFFER_NOTHIN where the new world is */
		for (int wx = topLeftCorner.first; wx != lowerRightCorner.first+1; ++wx)
		{
			for (int wy = topLeftCorner.second; wy != lowerRightCorner.second+1; ++wy)
			{
				world_[wx][wy].worldObjectType = BUFFER_NOTHING;
				world_[wx][wy].world = worldWithin;
			}
		}

		addWorldOffset(worldWithin, make_pair(topLeftCorner.first-(viewSize_/2), topLeftCorner.second-(viewSize_/2)));
		worldWithin->addWorldOffset(dist_ptr<World>(this), make_pair((viewSize_/2)-topLeftCorner.first, (viewSize_/2)-topLeftCorner.second));

		pair<int, int> topLeftCornerWithNothing = topLeftCorner;
		topLeftCornerWithNothing.first += viewSize_/2;
		topLeftCornerWithNothing.second += viewSize_/2;

		pair<int, int> lowerRightCornerWithNothing = lowerRightCorner;
		lowerRightCornerWithNothing.first -= viewSize_/2;
		lowerRightCornerWithNothing.second -= viewSize_/2;

		/* change the new world to BUFFER_VOID where we dont need to buffer */
		if ((topLeftCornerWithNothing.first <= lowerRightCornerWithNothing.first) &&
				(topLeftCornerWithNothing.second <= lowerRightCornerWithNothing.second))
		{
			for (int wx = topLeftCornerWithNothing.first; wx != lowerRightCornerWithNothing.first+1; ++wx)
			{
				for (int wy = topLeftCornerWithNothing.second; wy != lowerRightCornerWithNothing.second+1; ++wy)
				{
					world_[wx][wy].worldObjectType = BUFFER_VOID;
					world_[wx][wy].world = worldWithin;
				}
			}
		}

		return worldWithin;
	}

	void World::addWorldOffset(const ggg::dist_ptr<World>& world, const std::pair<int, int>& offset)
	{
		worldOffsets_[world] = offset;
	}

	std::pair<int, int> World::getPositionWithOffset(const ggg::dist_ptr<World>& world, const std::pair<int, int>& pos)
	{
		cWorldOffsetItr_t woItr = worldOffsets_.find(world);
		if (woItr == worldOffsets_.end())
		{
			return make_pair(-1, -1);
		}
		return make_pair(pos.first+worldOffsets_[world].first, pos.second+worldOffsets_[world].second);
	}
}

