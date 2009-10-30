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

add_subdirectory(ggg)

add_library(World STATIC World.cpp ggg/WorldMigration.cpp)
target_link_libraries(World Avatar Goldmine ProxyFactory)

add_library(Avatar STATIC Avatar.cpp ggg/AvatarMigration.cpp)
target_link_libraries(Avatar World Goldmine ProxyFactory)

add_library(View STATIC View.cpp)
target_link_libraries(View ProxyFactory)

add_library(Goldmine STATIC Goldmine.cpp ggg/GoldmineMigration.cpp)
target_link_libraries(Goldmine World ProxyFactory)

# Niflheim server, proxy and client
add_executable(client Client.cpp)
target_link_libraries(client Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

add_executable(mclient ManualClient.cpp)
target_link_libraries(mclient Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

add_executable(server Server.cpp)
target_link_libraries(server Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

add_executable(proxy Proxy.cpp)
target_link_libraries(proxy Ginnungagap NetAddr Uuid NameService ${CMAKE_THREAD_LIBS_INIT} ProxyFactory ObjectFactory Skeleton World Avatar View Goldmine)

