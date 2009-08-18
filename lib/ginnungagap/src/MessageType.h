/************************************************************************
*                                                                       *
*  Copyright (C) 2009 by Geir Erikstad                                  *
*  geirr@baldr.no                                                       *
*                                                                       *
*  This file is part of Ginnungagap.                                    *
*                                                                       *
*  Ginnungagap is free software: you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by *
*  the Free Software Foundation, either version 2 of the License, or    *
*  (at your option) any later version.                                  *
*                                                                       *
*  Ginnungagap is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
*  GNU General Public License for more details.                         *
*                                                                       *
*  You should have received a copy of the GNU General Public License    *
*  along with Ginnungagap. If not, see <http://www.gnu.org/licenses/>.  *
*                                                                       *
************************************************************************/

#ifndef GINNUNGAGAP_MESSAGETYPE_H
#define GINNUNGAGAP_MESSAGETYPE_H

#include "XdrReceiveBuffer.h"
#include "CommunicationSocket.h"

namespace ginnungagap
{
	enum MessageType
	{
		INIT,
		INITACK,
		MAININIT,
		PROXYINFO,
		PROXYINIT,
		NEED,
		DONTNEED,
		RMI,
		EVENT,
		FORWARDRMI,
		RMIACKRETVAL,
		MIGOBJ,
		MIGACK,
		NSUPDATE,
		NSREQ,
		LATINFO,
		MIGGRP,
		MIGSNTALL,
		MIGGRPACK,
		INITCHOBJGRP,
		ICOGACKISHERE,
		ICOGACKISNOTHERE,
		OBJGRPINFO,

		/* For messages from local Ginnungagap */
		DELCOMMSOCK,
		NONE
	};

	struct Message
	{
		XdrReceiveBuffer* xdr;
		CommunicationSocket* commSock;
		MessageType messageType;
	};

	enum RmiAckValues
	{
		NEWADDR,
		RMIOK,
		RMIOKWNEWADDR,
		UNKNOWN
	};
}

#endif

