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

#include "View.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

using std::vector;
using std::pair;
using std::make_pair;

namespace niflheim
{
	typedef vector< WorldObjectInfo >::const_iterator cObjItr_t;

	void View::updateView(const AvatarsView& avatarsView)
	{
		resetView();
		for (int x = avatarsView.visibleTopLeftCorner.first; x != avatarsView.visibleLowerRightCorner.first; ++x)
		{
			for (int y = avatarsView.visibleTopLeftCorner.second; y != avatarsView.visibleLowerRightCorner.second; ++y)
			{
				view_[x][y] = NOTHING;
			}
		}
		for (cObjItr_t object = avatarsView.worldObjects.begin(); object != avatarsView.worldObjects.end(); ++object)
		{
			view_[object->position.first][object->position.second] = object->worldObjectType;
		}
		drawView();
	}

	void View::drawView() const
	{
		cout << "+";
		for (unsigned int i = 0; i != view_[0].size(); ++i)
		{
			cout << "--";
		}
		cout << "-+";
		cout << endl;
		for (unsigned int x = 0; x != view_.size(); ++x)
		{
			cout << "|";
			for (unsigned int y = 0; y != view_[0].size(); ++y)
			{
				if (view_[x][y] == AVATAR)
				{
					cout << " A";
				}
				else if (view_[x][y] == GOLDMINE)
				{
					cout << " G";
				}
				else if (view_[x][y] == VOID)
				{
					cout << " *";
				}
				else if(view_[x][y] == NOTHING)
				{
					cout << " .";
				}
				else
				{
					cerr << "Error" << endl;
					exit(0);
				}
			}
			cout << " |" << endl;
		}
		cout << "+";
		for (unsigned int i = 0; i != view_[0].size(); ++i)
		{
			cout << "--";
		}
		cout << "-+";
		cout << endl;
	}

	void View::resetView()
	{
		for (unsigned int x = 0; x != view_.size(); ++x)
		{
			for (unsigned int y = 0; y != view_[0].size(); ++y)
			{
				view_[x][y] = VOID;
			}
		}
	}

	int View::getViewSize() const
	{
		return view_.size();
	}

	bool View::isFree(const pair<int, int>& coords) const
	{
		if (coords.first < 0 || coords.first >= getViewSize() || coords.second < 0 || coords.second >= getViewSize())
		{
			return false;
		}
		return (view_[coords.first][coords.second] == NOTHING);
	}

	bool View::noneFree() const
	{
		pair<int, int> coords = myPosition();
		if (isFree(make_pair(coords.first-1, coords.second)))
		{
			return false;
		}
		else if (isFree(make_pair(coords.first+1, coords.second)))
		{
			return false;
		}
		else if (isFree(make_pair(coords.first, coords.second-1)))
		{
			return false;
		}
		else if (isFree(make_pair(coords.first, coords.second+1)))
		{
			return false;
		}
		return true;
	}

	pair<int, int> View::myPosition() const
	{
		return make_pair(getViewSize()/2, getViewSize()/2);
	}

	bool View::directionIsClear(const Direction& direction) const
	{
		pair<int, int> position = myPosition();
		if (direction == NORTH)
		{
			--position.first;
		}
		else if (direction == SOUTH)
		{
			++position.first;
		}
		else if (direction == WEST)
		{
			--position.second;
		}
		else if (direction == EAST)
		{
			++position.second;
		}
		else
		{
			/* we have tried to move in direction NONE */
			return true;
		}
		return isFree(position);
	}

	pair<int, int> View::getGoldmineCoords() const
	{
		for (unsigned int x = 0; x != view_.size(); ++x)
		{
			for (unsigned int y = 0; y != view_[0].size(); ++y)
			{
				if (view_[x][y] == GOLDMINE)
				{
					return make_pair(x, y);
				}
			}
		}
		return make_pair(-1, -1);
	}

	pair<Direction, Direction> View::getDirectionToGoldmine() const
	{
		pair<int, int> coords = getGoldmineCoords();
		if (make_pair(-1, -1) == coords)
		{
			return make_pair(NONE, NONE);
		}

		pair<Direction, Direction> directions;
		pair<int, int> myCoords = myPosition();
		if (myCoords.first > coords.first)
		{
			directions.first = NORTH;
		}
		else if (myCoords.first == coords.first)
		{
			directions.first = NONE;
		}
		else if (myCoords.first < coords.first)
		{
			directions.first = SOUTH;
		}

		if (myCoords.second > coords.second)
		{
			directions.second = WEST;
		}
		else if (myCoords.second == coords.second)
		{
			directions.second = NONE;
		}
		else if (myCoords.second < coords.second)
		{
			directions.second = EAST;
		}
		return directions;
	}

	bool View::isNextToGoldmine() const
	{
		pair<int, int> coords = getGoldmineCoords();
		if (make_pair(-1, -1) == coords)
		{
			return false;
		}
		pair<int, int> myCoords = myPosition();
		if (!((myCoords.first == coords.first) || (myCoords.first - 1 == coords.first) || (myCoords.first + 1 == coords.first)))
				return false;
		if (!((myCoords.second == coords.second) || (myCoords.second - 1 == coords.second) || (myCoords.second + 1 == coords.second)))
				return false;
		return true;
	}
}

