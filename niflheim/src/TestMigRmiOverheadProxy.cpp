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

using std::cerr; using std::endl; using std::string; using std::cin;
using std::cout; using std::stringstream; using std::istringstream;

using ginnungagap::Ginnungagap; using ginnungagap::NetAddr; using ginnungagap::Uuid;
using ginnungagap::PROXY;

int main(int argc, char* argv[])
{
	/* get the NetAddr of the server */
	NetAddr netAddr(argv[1]);
	NetAddr server(argv[2]);

	/* get times to migrate */
	int numberOfTimesToMigrateGroup;
	istringstream ss(argv[3]);
	ss >> numberOfTimesToMigrateGroup;

	/* Initialize Server with our NetAddr */
	Ginnungagap::Initialize(PROXY, 50, netAddr, server);

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

	Uuid testId("00000000-0000-0000-0000-000000000001");

	cout << "Press any key to start migrating group if local" << endl;
	cin.get();

	while (numberOfTimesToMigrateGroup != 0)
	{
		if (Ginnungagap::Instance()->nameService()->isLocal(testId) && Ginnungagap::Instance()->migrationService()->getMigrationGroup(testId) != 0 &&
				Ginnungagap::Instance()->migrationService()->getMigrationGroup(testId)->isActive())
		{
			Ginnungagap::Instance()->migrationService()->getMigrationGroup(testId)->migrateGroup(server);
			--numberOfTimesToMigrateGroup;
		}
		usleep(700000);
	}

	cout << "Done ping pong migrating, press any key to exit" << endl;
	cin.get();

	/* restore original terminal mode */
	ioctl(0, TCSETS, &oldT);

	return 1;
}

