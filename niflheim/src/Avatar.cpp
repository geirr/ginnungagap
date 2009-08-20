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

#include "Avatar.h"
#include "World.h"

#include <string>


#include <iostream>
using std::cerr;
using std::endl;

using ggg::dist_ptr;

namespace niflheim
{
	void Avatar::move(const Direction& direction)
	{
		if (active_)
		{
			world_->moveAvatar(dist_ptr<Avatar>(this), direction);
		}
	}

	void Avatar::updateView(const AvatarsView& avatarsView)
	{
		view_->updateView(avatarsView);
	}

	void Avatar::changeWorld(const dist_ptr<World>& newWorld)
	{
		world_ = newWorld;
	}

	void Avatar::deactivate()
	{
		active_ = false;
	}

	void Avatar::activate()
	{
		active_ = true;
	}

	void Avatar::deleteAvatar()
	{
		world_->deleteAvatar(dist_ptr<Avatar>(this));
	}
}

