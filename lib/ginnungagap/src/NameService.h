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

#ifndef GINNUNGAGAP_NAMESERVICE_H
#define GINNUNGAGAP_NAMESERVICE_H

#include "NetAddr.h"
#include "Uuid.h"
#include "NameServiceRecord.h"

#include <map>

namespace ggg
{
	class Object;
	class XdrReceiveBuffer;
	class XdrSendBuffer;

	class NameService
	{
		public:
			NameService() {}
			~NameService() {}

			bool exists(const Uuid& objectId) const;
			bool isLocal(const Uuid& objectId) const;

			Object* getLocalObjectOrProxy(const Uuid& objectId) const;	
			void setLocalObjectOrProxy(Object* objectOrProxy);

			NetAddr netAddr(const Uuid& objectId) const;

			void bind(Object& object);
			void setAsRemote(const Uuid& objectId, const NetAddr& netAddr);

			void addRemoteObject(const Uuid& objectId, const NetAddr& netAddr);

			void updateNetAddr(const Uuid& objectId, const NetAddr& netAddr);

			void deleteNameServiceRecord(const Uuid& objectId);

			XdrSendBuffer* getNsRecordUpdate(const Uuid& objectId) const;
			void updateNsRecord(XdrReceiveBuffer* xdrBuffer);

			void increaseProxyCount(const Uuid& objectId);
			void decreaseProxyCount(const Uuid& objectId);

		private:
			std::map<Uuid, NameServiceRecord> nameServiceRegister_;
	};
}

#endif

