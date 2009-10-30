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

#ifndef NIFLHEIM_WORLD_H
#define NIFLHEIM_WORLD_H

#include <XdrReceiveBuffer.h>
#include <Uuid.h>
#include <dist_ptr.h>

#include "Avatar.h"
#include "View.h"
#include "Goldmine.h"
#include "AvatarsView.h"

#include <cstdlib> 
#include <ctime> 

#include <vector>
#include <map>
#include <list>
#include <utility>

namespace niflheim
{
	class World;

	struct WorldObject
	{
		WorldObjectType worldObjectType;
		ggg::dist_ptr<Avatar> avatar;
		ggg::dist_ptr<Goldmine> goldmine;
		ggg::dist_ptr<World> world;
	};

	class World : public ggg::Object
	{
		public:
			World(const int& xLen, const int& yLen, const int& viewSize,
					const ggg::Uuid& objectId = ggg::Uuid(),
					const ggg::dist_ptr<World>& worldAround = ggg::dist_ptr<World>(ggg::Uuid("00000000-0000-0000-0000-000000000000")));

			/* These should be callable with rmi */
			virtual ggg::dist_ptr<Avatar> getNewAvatar(const ggg::dist_ptr<View>& view);
			virtual std::pair<int, int> getSize();
			virtual int getViewSize();

			/* events */
			virtual void moveAvatar(ggg::dist_ptr<Avatar> avatar, const Direction& direction);
			virtual void updateWithGoldmine(const ggg::dist_ptr<World>& world, const std::pair<int, int>& position);
			virtual void updateWithAvatar(const ggg::dist_ptr<World>& world, const std::pair<int, int>& position);
			virtual void removeBufferAvatar(const ggg::dist_ptr<World>& world, const std::pair<int, int>& position);
			virtual void moveBufferAvatar(const ggg::dist_ptr<World>& world, const std::pair<int, int>& position, const Direction& direction);
			virtual void moveFromOtherWorld(const ggg::dist_ptr<Avatar>& avatar, ggg::dist_ptr<World> world, const std::pair<int, int>& from, const std::pair<int, int>& to);
			virtual void moveOK(const std::pair<int, int>& from, const std::pair<int, int>& to);
			virtual void moveNotOK(const std::pair<int, int>& from);
			virtual void deleteAvatar(const ggg::dist_ptr<Avatar>& avatar);

			/* Only callable localy */
			void drawWorld() const;
			void addGoldmine(const std::pair<int, int>& goldmineCoords);
			ggg::dist_ptr<World> addWorldWithin(const std::pair<int, int>& topLeftCorner, const std::pair<int, int>& lowerRightCorner, const ggg::Uuid& objectId = ggg::Uuid());
			void addWorldOffset(const ggg::dist_ptr<World>& world, const std::pair<int, int>& offset);

			/* migration */
			ggg::XdrSendBuffer* deflate();
			World(ggg::XdrReceiveBuffer* xdr);
			World() {}

		private:
			bool isFree(const std::pair<int, int>& position) const;
			std::pair<int, int> getPosition(const ggg::dist_ptr<Avatar>& avatar) const;
			std::pair<int, int> getPosition(const ggg::dist_ptr<Goldmine>& goldmine) const;
			void updateAffectedAvatarsViews(const std::pair<int, int>& position, const Direction& direction = NONE);
			std::list< ggg::dist_ptr<Avatar> > getAvatarsInLOS(const std::pair<int, int>& position, const Direction& direction = NONE);
			std::list< ggg::dist_ptr<World> > getWorldsInLOS(const std::pair<int, int>& position, const Direction& direction = NONE);
			AvatarsView getAvatarsView(const ggg::dist_ptr<Avatar>& avatar);
			std::pair<int, int> getPositionWithOffset(const ggg::dist_ptr<World>& world, const std::pair<int, int>& pos);

			/* data to migrate */
			std::vector< std::vector< WorldObject > > world_;
			unsigned int viewSize_;
			ggg::Uuid migrationGroupId_;
			std::map< ggg::dist_ptr<World>, std::pair<int, int> > worldOffsets_;
	};
}

#endif

