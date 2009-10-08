#########################################################################
#                                                                       #
#  Copyright (C) 2009 by Geir Erikstad                                  #
#  geirr@baldr.no                                                       #
#                                                                       #
#  This file is part of Niflheim.                                       #
#                                                                       #
#  Niflheim is free software: you can redistribute it and/or modify     #
#  it under the terms of the GNU General Public License as published by #
#  the Free Software Foundation, either version 2 of the License, or    #
#  (at your option) any later version.                                  #
#                                                                       #
#  Niflheim is distributed in the hope that it will be useful,          #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of       #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         #
#  GNU General Public License for more details.                         #
#                                                                       #
#  You should have received a copy of the GNU General Public License    #
#  along with Niflheim. If not, see <http://www.gnu.org/licenses/>.     #
#                                                                       #
#########################################################################

find_package(Threads REQUIRED)

#set(CURSES_NEED_NCURSES TRUE)
#find_package(Curses REQUIRED)

#  CURSES_LIBRARIES - The libraries needed to use Curses

add_subdirectory(ggg)

add_library(World STATIC World.cpp ggg/WorldMigration.cpp)
target_link_libraries(World Avatar Goldmine ProxyFactory)

add_library(Avatar STATIC Avatar.cpp ggg/AvatarMigration.cpp)
target_link_libraries(Avatar World Goldmine ProxyFactory)

add_library(View STATIC View.cpp)
target_link_libraries(View ProxyFactory)

add_library(Goldmine STATIC Goldmine.cpp ggg/GoldmineMigration.cpp)
target_link_libraries(Goldmine World ProxyFactory)

add_library(TestObject STATIC TestObject.cpp ggg/TestObjectMigration.cpp)


# Niflheim server, proxy and client
add_executable(client Client.cpp)
target_link_libraries(client Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

add_executable(mclient ManualClient.cpp)
target_link_libraries(mclient Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

add_executable(server Server.cpp)
target_link_libraries(server Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

add_executable(proxy Proxy.cpp)
target_link_libraries(proxy Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

# Client that only measures latency
#add_executable(mlclient test/ClientMeasureLat.cpp)
#target_link_libraries(mlclient Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

# Test server and proxy for mig and mig grp tests
#add_executable(testmigserver test/TestMigServer.cpp)
#target_link_libraries(testmigserver Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testmiggrpserver test/TestMigGrpServer.cpp)
#target_link_libraries(testmiggrpserver Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testmigproxy test/TestMigProxy.cpp)
#target_link_libraries(testmigproxy Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

# Test server, proxy and client for rmi overhead during migration
#add_executable(testmroserver test/TestMigRmiOverheadServer.cpp)
#target_link_libraries(testmroserver Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testmroproxy test/TestMigRmiOverheadProxy.cpp)
#target_link_libraries(testmroproxy Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testmroclient test/TestMigRmiOverheadClient.cpp)
#target_link_libraries(testmroclient Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

# Test server and client for rmi, and lmi test
#add_executable(testclient test/TestRmiClient.cpp)
#target_link_libraries(testclient Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testlmiclient test/TestRmiLocalClient.cpp)
#target_link_libraries(testlmiclient Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testrmiserver test/TestRmiServer.cpp)
#target_link_libraries(testrmiserver Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

#add_executable(testlmi test/TestLMI.cpp)
#target_link_libraries(testlmi Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

