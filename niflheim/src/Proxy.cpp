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
#include "TestObject.h"

#include <pthread.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <sstream>

using std::cerr; using std::cout; using std::endl; using std::istringstream;

using ggg::Ginnungagap; using ggg::Uuid; using ggg::dist_ptr; using ggg::NetAddr;
using ggg::PROXY;

int main(int argc, char* argv[])
{
	NetAddr thisAddr(argv[1]);
	NetAddr mainServAddr(argv[2]);

	int coreSelThreshold;
	int coreSelInterval;

	istringstream ss1(argv[3]);
	istringstream ss2(argv[4]);
	ss1 >> coreSelThreshold;
	ss2 >> coreSelInterval;

	/* Initialize Server */
	Ginnungagap::Initialize(PROXY, 150, thisAddr, mainServAddr, coreSelThreshold, coreSelInterval);

	/* Start necessary threads */
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();

	/* Run last loop in this thread */
	Ginnungagap::Instance()->messageHandlerWithCoreSelection();
}

