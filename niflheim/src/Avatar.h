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

#ifndef NIFLHEIM_AVATAR_H
#define NIFLHEIM_AVATAR_H

#include <Object.h>
#include <XdrReceiveBuffer.h>
#include <dist_ptr.h>

#include "Direction.h"
#include "AvatarsView.h"

#include <cstdlib> 
#include <ctime> 

namespace niflheim
{
	class World;
	class View;

	class Avatar : public ginnungagap::Object
	{
		public:
			Avatar(const ginnungagap::dist_ptr<World>& world, const ginnungagap::dist_ptr<View>& view) :
				world_(world), view_(view), active_(true) {}

			/* RMI */
			virtual void move(const Direction& direction);
			virtual void deleteAvatar();

			/* events */
			virtual void updateView(const AvatarsView& avatarsView);
			virtual void changeWorld(const ginnungagap::dist_ptr<World>& newWorld);
			virtual void deactivate();
			virtual void activate();

			/* migration */
			ginnungagap::XdrSendBuffer* deflate();
			Avatar(ginnungagap::XdrReceiveBuffer* xdr);
			Avatar() {}

		private:
			ginnungagap::dist_ptr<World> world_;
			ginnungagap::dist_ptr<View> view_;
			/* not allowd to move more, if we are trying to move between worlds */
			bool active_;
	};
}

#endif

