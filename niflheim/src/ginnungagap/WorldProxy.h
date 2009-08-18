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

#ifndef GINNUNGAGAP_WORLDPROXY_H
#define GINNUNGAGAP_WORLDPROXY_H

#include <Proxy.h>

#include "World.h"
#include "Uuid.h"

namespace ginnungagap
{
	class WorldProxy : public Proxy, public niflheim::World
	{
		public:
			WorldProxy(const Uuid& objectId);
			~WorldProxy();

			/* RMI callable */
			ginnungagap::dist_ptr<niflheim::Avatar> getNewAvatar(const ginnungagap::dist_ptr<niflheim::View>& view);
			std::pair<int, int> getSize();
			int getViewSize();

			/* events */
			void moveAvatar(ginnungagap::dist_ptr<niflheim::Avatar> avatar, const niflheim::Direction& direction);
			void updateWithGoldmine(const ginnungagap::dist_ptr<niflheim::World>& world, const std::pair<int, int>& position);
			void updateWithAvatar(const ginnungagap::dist_ptr<niflheim::World>& world, const std::pair<int, int>& position);
			void removeBufferAvatar(const ginnungagap::dist_ptr<niflheim::World>& world, const std::pair<int, int>& position);
			void moveBufferAvatar(const ginnungagap::dist_ptr<niflheim::World>& world, const std::pair<int, int>& position, const niflheim::Direction& direction);
			void moveFromOtherWorld(const ginnungagap::dist_ptr<niflheim::Avatar>& avatar, ginnungagap::dist_ptr<niflheim::World> world, const std::pair<int, int>& from, const std::pair<int, int>& to);
			void moveOK(const std::pair<int, int>& from, const std::pair<int, int>& to);
			void moveNotOK(const std::pair<int, int>& from);
			void deleteAvatar(const ginnungagap::dist_ptr<niflheim::Avatar>& avatar);
	};
}

#endif

