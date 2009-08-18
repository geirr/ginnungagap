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

#ifndef GINNUNGAGAP_AVATARPROXY_H
#define GINNUNGAGAP_AVATARPROXY_H

#include <Proxy.h>

#include "Avatar.h"
#include "Uuid.h"

namespace ginnungagap
{
	class AvatarProxy : public Proxy, public niflheim::Avatar
	{
		public:
			AvatarProxy(const Uuid& objectId);
			~AvatarProxy();

			/* Event */
			void updateView(const niflheim::AvatarsView& avatarsView);
			void move(const niflheim::Direction& direction);
			void changeWorld(const ginnungagap::dist_ptr<niflheim::World>& newWorld);
			void deactivate();
			void activate();
			void deleteAvatar();
	};
}

#endif

