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

#ifndef GINNUNGAGAP_NAMESERVICERECORD_H
#define GINNUNGAGAP_NAMESERVICERECORD_H

#include "NetAddr.h"
#include "Uuid.h"

namespace ggg
{
	class Object;

	class NameServiceRecord
	{
		public:
			/* Local constructor */
			NameServiceRecord(const Uuid& objectId, Object* object);
			/* Remote constructor */
			NameServiceRecord(const Uuid& objectId, const NetAddr& remoteAddr);

			bool isLocal() const;

			Uuid objectId() const;

			Object* localObjectOrProxy() const;
			void setLocalObjectOrProxy(Object* localObjectOrProxy);

			void increaseProxyCount();
			void decreaseProxyCount();

			NetAddr netAddr() const;
			void updateNetAddr(const NetAddr& netAddr);

			void setToLocal(Object* object);
			void setToRemote(const NetAddr& remoteAddr);
		private:
			const Uuid objectId_;
			NetAddr netAddr_;
			int proxyCount_;
			Object* localObjectOrProxy_;
	};
}

#endif
