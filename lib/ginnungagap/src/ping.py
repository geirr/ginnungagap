#!/usr/bin/env python

"""
Copyright (C) 2009 by Geir Erikstad
geirr@baldr.no

This file is part of Ginnungagap.

Ginnungagap is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Ginnungagap is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ginnungagap. If not, see <http://www.gnu.org/licenses/>.
"""

import re, sys
from subprocess import *

hostRegEx = re.compile('(.+):')

hostToPing = hostRegEx.match(sys.argv[1]).group(1)

ping = Popen(['ping', '-c 5', hostToPing], stdout=PIPE)
output = ping.communicate()[0]
ping.wait()

p = re.compile('time=(\d+\.{0,1}\d*)')

stringlatencies = p.findall(output)

if not stringlatencies:
	print "0"
else:
	latencies = []
	for strnumb in stringlatencies:
		latencies.append(float(strnumb))
	latencies.sort()
	print int(round(latencies[2]))

