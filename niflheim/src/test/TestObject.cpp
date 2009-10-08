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

#include "TestObject.h"
#include "Object.h"

#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;


void TestObject::void_void()
{
	for (int i = 0; i != 100000; ++i)
	{
	}
}

void TestObject::void_int(int one)
{
	for (int i = 0; i != 100000; ++i)
	{
	}
}

int TestObject::int_void()
{
	for (int i = 0; i != 100000; ++i)
	{
	}
	return 0;
}

int TestObject::int_int(int one)
{
	for (int i = 0; i != 100000; ++i)
	{
	}
	return 1;
}

int TestObject::int_intInt(int one, int two)
{
	for (int i = 0; i != 100000; ++i)
	{
	}
	return 2;
}

int TestObject::int_intIntInt(int one, int two, int three)
{
	for (int i = 0; i != 100000; ++i)
	{
	}
	return 3;
}

int TestObject::int_vectorOfInts(const std::vector<int>& vectorOfInts)
{
	for (int i = 0; i != 100000; ++i)
	{
	}
	return 4;
}

