#########################################################################
#                                                                       #
#  Copyright (C) 2009 by Geir Erikstad                                  #
#  geirr@baldr.no                                                       #
#                                                                       #
#  This file is part of Ginnungagap.                                    #
#                                                                       #
#  Ginnungagap is free software: you can redistribute it and/or modify  #
#  it under the terms of the GNU General Public License as published by #
#  the Free Software Foundation, either version 2 of the License, or    #
#  (at your option) any later version.                                  #
#                                                                       #
#  Ginnungagap is distributed in the hope that it will be useful,       #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of       #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         #
#  GNU General Public License for more details.                         #
#                                                                       #
#  You should have received a copy of the GNU General Public License    #
#  along with Ginnungagap. If not, see <http://www.gnu.org/licenses/>.  #
#                                                                       #
#########################################################################

find_package(Uuid REQUIRED)
find_package(XDR REQUIRED)
find_package(Boost 1.34.0 REQUIRED COMPONENTS graph)
find_package(Threads REQUIRED)

set(NetAddrSRCS
	NetAddr.cpp
)

set(UuidSRCS
	Uuid.cpp
)

set(ObjectSRCS
	Object.cpp
)

set(NameServiceSRCS
	NameServiceRecord.cpp 
	NameService.cpp
)

set(XdrSRCS
	XdrBuffer.cpp
	XdrSendBuffer.cpp
	XdrReceiveBuffer.cpp
)

set(SocketSRCS
	Socket.cpp
	CommunicationSocket.cpp
	ListenSocket.cpp
)

set(CommunicationSocketManagerSRCS
	CommunicationSocketManager.cpp
)

set(MigrationServiceSRCS
	MigrationService.cpp
	MigrationGroup.cpp
)

set(GinnungagapSRCS
	Ginnungagap.cpp
	MessageHandler.cpp
)

set(CoreSelSRCS
	NetworkGraph.cpp
)

set(ProxySRCS
	Proxy.cpp
)

add_library(Object STATIC ${ObjectSRCS})
target_link_libraries(Object Uuid)

add_library(NetAddr STATIC ${NetAddrSRCS})

add_library(Uuid STATIC ${UuidSRCS})
target_link_libraries(Uuid ${Uuid_LIBRARY})

add_library(NameService STATIC ${NameServiceSRCS})
target_link_libraries(NameService Object Uuid NetAddr Xdr)

add_library(Xdr STATIC ${XdrSRCS})
target_link_libraries(Xdr Uuid NetAddr ${XDR_LIBRARY})

add_library(CommunicationSocketManager STATIC ${CommunicationSocketManagerSRCS})
target_link_libraries(CommunicationSocketManager Socket NetAddr)

add_library(Socket STATIC ${SocketSRCS})
target_link_libraries(Socket NetAddr Xdr CommunicationSocketManager)

add_library(Ginnungagap STATIC ${GinnungagapSRCS})
target_link_libraries(Ginnungagap Xdr Socket MigrationService NetworkGraph NameService ProxyFactory Skeleton)

add_library(MigrationService STATIC ${MigrationServiceSRCS})
target_link_libraries(MigrationService NetAddr Socket CommunicationSocketManager NameService Object)

add_library(NetworkGraph STATIC ${CoreSelSRCS})
target_link_libraries(NetworkGraph NetAddr)

add_library(Proxy STATIC ${ProxySRCS})
target_link_libraries(Proxy Xdr Ginnungagap)

add_executable(testxdr test.cpp)
target_link_libraries(testxdr Xdr)

