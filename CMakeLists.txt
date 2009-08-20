##########################################################################
#  Copyright (C) 2009 by Geir Erikstad                                   #
#  geirr@baldr.no                                                        #
#                                                                        #
#  This file is part of Niflheim.                                        #
#                                                                        #
#  Niflheim is free software: you can redistribute it and/or modify      #
#  it under the terms of the GNU General Public License as published by  #
#  the Free Software Foundation, either version 3 of the License, or     #
#  (at your option) any later version.                                   #
#                                                                        #
#  Niflheim is distributed in the hope that it will be useful,           #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#  GNU General Public License for more details.                          #
#                                                                        #
#  You should have received a copy of the GNU General Public License     #
#  along with Niflheim.  If not, see <http://www.gnu.org/licenses/>.     #
#                                                                        #
##########################################################################

project(Niflheim)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

#set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions(-Wall -O2 -Wno-deprecated)

include_directories("lib/ginnungagap/src")
include_directories("niflheim/src/ggg")
include_directories("niflheim/src")

add_subdirectory(lib)
add_subdirectory(niflheim)


