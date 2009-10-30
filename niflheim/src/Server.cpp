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
#include <MigrationService.h>

#include "World.h"
#include "Avatar.h"

#include <pthread.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <sstream>

using std::cerr; using std::cout; using std::endl; using std::string;
using std::make_pair; using std::istringstream;

using ggg::Ginnungagap; using ggg::NetAddr; using ggg::dist_ptr;
using ggg::Uuid; using ggg::SERVER;

using niflheim::World;

int main(int argc, char* argv[])
{
	/* get the NetAddr of the server */
	NetAddr netAddr(argv[1]);

	int coreSelThreshold;
	int coreSelInterval;

	istringstream ss1(argv[2]);
	istringstream ss2(argv[3]);
	ss1 >> coreSelThreshold;
	ss2 >> coreSelInterval;

	/* Initialize Server with our NetAddr */
	Ginnungagap::Initialize(SERVER, 150, netAddr, netAddr, coreSelThreshold, coreSelInterval);

	/* view size for the clients */
	const int viewSize = 11;

	/* Create the world! */
	/* We only have one world, and its the startingpoint of all clients, thus we set it's id to 1 */
	/* view size must be an odd number */
	dist_ptr<World> theWorld = new World(40, 40, viewSize, Uuid("00000000-0000-0000-0000-000000000001"));

	/* Start necessary threads */
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();

	/* Run last loop in this thread */
	Ginnungagap::Instance()->messageHandlerWithCoreSelection();
}

