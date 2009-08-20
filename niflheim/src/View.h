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

#ifndef NIFLHEIM_VIEW_H
#define NIFLHEIM_VIEW_H

#include <Object.h>

#include <XdrSendBuffer.h>

#include <vector>
#include <utility>

#include "WorldObjects.h"
#include "AvatarsView.h"
#include "Direction.h"

namespace niflheim
{
	/* Need to support rmi, but not migration */
	class View : public ggg::Object
	{
		public:
			View() {}
			View(const int& viewSize) : view_(viewSize, std::vector<WorldObjectType>(viewSize)) {}

			/* Should be callable with event */
			virtual void updateView(const AvatarsView& avatarsView);

			/* local functions */
			bool directionIsClear(const Direction& direction) const;
			int getViewSize() const;
			bool isFree(const std::pair<int, int>& coords) const;
			bool noneFree() const;
			std::pair<int, int> myPosition() const;
			std::pair<int, int> getGoldmineCoords() const;
			std::pair<Direction, Direction> getDirectionToGoldmine() const;
			bool isNextToGoldmine() const;

			/* Object will complain if this is not included
			 * Might devide object in two later */
			ggg::XdrSendBuffer* deflate() { return new ggg::XdrSendBuffer(ggg::INT); }

		private:
			void drawView() const;
			void resetView();

			std::vector< std::vector< WorldObjectType > > view_;
	};
}

#endif

