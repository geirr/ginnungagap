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

#include <Ginnungagap.h>
#include <NameService.h>
#include <dist_ptr.h>

#include "World.h"
#include "Avatar.h"
#include "View.h"

#include <pthread.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <sstream>

using std::cout; using std::endl; using std::pair; using std::istringstream; using std::srand; using std::rand;
using std::cerr; using std::stringstream;

using ggg::Ginnungagap; using ggg::dist_ptr; using ggg::NetAddr;
using ggg::Uuid; using ggg::CLIENT;

using niflheim::World; using niflheim::Avatar; using niflheim::View; using niflheim::NONE;
using niflheim::Direction; using niflheim::NORTH; using niflheim::WEST; using niflheim::EAST; using niflheim::SOUTH;

int main(int argc, char* argv[])
{
	NetAddr thisAddr(argv[1]);
	NetAddr mainServAddr(argv[2]);

	int numberOfMoves;
	istringstream ss(argv[3]);
	ss >> numberOfMoves;

	srand((unsigned)time(0));

	/* Initialize Server */
	Ginnungagap::Initialize(CLIENT, 10, thisAddr, mainServAddr);

	/* Start threads */
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startMessageHandlerThread();
	Ginnungagap::Instance()->startLatencyMesurerThread();

	/* Start looking for the world at the main server */
	Ginnungagap::Instance()->nameService()->addRemoteObject(Uuid("00000000-0000-0000-0000-000000000001"), mainServAddr);
	
	/* Get a pointer to the world */
	dist_ptr<World> theWorld = dist_ptr<World>(Uuid("00000000-0000-0000-0000-000000000001"));

	/* Create a world view */
	int viewSize = theWorld->getViewSize();
	dist_ptr<View> view = new View(viewSize);
	Ginnungagap::Instance()->nameService()->bind(*view);

	/* Get us a new Avatar */
	dist_ptr<Avatar> avatar = theWorld->getNewAvatar(view);

	/* reset the world pointer, since we dont need the world object anymore */
	theWorld.reset();

	/* Get a random general direction */
	Direction generalDirection = (Direction) (rand()%4);
	Direction currentDirection;

	int moves = 0;

	struct timeval before;
	struct timeval after;
	stringstream time;
	int retValue;

	/* Move randomly around */
	while (moves != numberOfMoves)
	{
		if (view->noneFree())
		{
			usleep(500000);
			continue;
		}

		/* Should we get a new general direction? */
		if (rand()%100 > 95 || !view->directionIsClear(generalDirection))
		{
			generalDirection = (Direction) (rand()%4);
		}

		/* go left or right? */
		if (rand()%100 > 80)
		{
			if (rand()%100 > 50)
			{
				/* left */
				if (generalDirection == 0)
				{
					currentDirection = (Direction) 3;
				}
				else
				{
					currentDirection = (Direction) (generalDirection-1);
				}
			}
			else
			{
				/* right */
				currentDirection = (Direction) ((generalDirection+1)%4);
			}
		}
		else
		{
			/* go in general direction */
			currentDirection = generalDirection;
		}

		if (view->directionIsClear(currentDirection))
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			avatar->move(currentDirection);
			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			time.str("");
			time << "move took: ";
			time << (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			Ginnungagap::Instance()->writeToLog(time.str());
			++moves;
			usleep(500000);
		}
	}

	/* delete the avatar */
	avatar->deleteAvatar();
}

