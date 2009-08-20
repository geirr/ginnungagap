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

#ifndef GINNUNGAGAP_OBJECT_H
#define GINNUNGAGAP_OBJECT_H

#include "Uuid.h"

namespace ggg
{
	class XdrSendBuffer;

	class Object
	{
		public:
			enum ObjectState
			{
				NORMAL,
				MIGRATING
			};

			Object() : objectId_(Uuid("00000000-0000-0000-0000-000000000000")), objectState_(NORMAL) {}
			Object(const Uuid& objectId) : objectId_(objectId), objectState_(NORMAL) {}
			virtual ~Object();

			bool hasObjectId() const;
			void generateObjectId();
			void setObjectId(const Uuid& objectId);
			Uuid objectId() const;

			ObjectState objectState() const;
			void setObjectState(const ObjectState& objectState);

			virtual XdrSendBuffer* deflate() = 0;

		private:
			Uuid objectId_;
			ObjectState objectState_;
	};
}

#endif

