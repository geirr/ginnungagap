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

#include "TestObject.h"

#include <pthread.h>
#include <string>
#include <iostream>
#include <sstream>
#include <errno.h>

#include <termio.h>

using std::cout; using std::cin; using std::endl; using std::string;
using std::cerr; using std::stringstream; using std::istringstream;

using ggg::Ginnungagap; using ggg::NetAddr; using ggg::Uuid;
using ggg::SERVER; using ggg::MigrationGroup;

int main(int argc, char* argv[])
{
	/* get the NetAddr of the server */
	NetAddr netAddr(argv[1]);

	/* get netaddr to the proxy */
	NetAddr proxy(argv[2]);

	/* get size of objects */
	int objectSize;
	istringstream ss(argv[3]);
	ss >> objectSize;

	/* get size of object groups */
	int groupSize;
	istringstream ss2(argv[4]);
	ss2 >> groupSize;

	/* Initialize Server with our NetAddr */
	Ginnungagap::Initialize(SERVER, 50, netAddr, netAddr);

	/* Start necessary threads */
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();
	Ginnungagap::Instance()->startMessageHandlerThread();

	/* set terminal in ``any key''-mode */
	struct termios oldT, newT;
	ioctl(0, TCGETS, &oldT);
	newT = oldT;
	newT.c_lflag &= ~ECHO;
	newT.c_lflag &= ~ICANON;
	ioctl(0, TCSETS, &newT);

	cout << "Press any key once proxy is connected" << endl;
	cin.get();
	cout << "Migrating object groups..." << endl;


	TestObject* test;
	MigrationGroup* testGrp;

	for (int i = 0; i != 1000; ++i)
	{
		testGrp = new MigrationGroup();
		for (int j = 0; j != groupSize; ++j)
		{
			test = new TestObject(objectSize);
			Ginnungagap::Instance()->nameService()->bind(*test);
			testGrp->addObject(test->objectId());
		}

		Ginnungagap::Instance()->migrationService()->addMigrationGroup(testGrp);
		testGrp->migrateGroup(proxy);

		usleep(500000);
	}

	cout << "Done migrating objects, press any key to exit" << endl;
	cin.get();

	/* restore original terminal mode */
	ioctl(0, TCSETS, &oldT);
	return 1;
}

