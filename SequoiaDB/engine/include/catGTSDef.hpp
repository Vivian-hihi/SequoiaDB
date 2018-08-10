/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = catGTSDef.hpp

   Descriptive Name = GTS definition

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/13/2018  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef CAT_GTS_DEF_HPP_
#define CAT_GTS_DEF_HPP_

#define GTS_SYS_COLLECTION_SPACE_NAME        "SYSGTS"

// SEQUENCE

#define CAT_SEQUENCE_NAME              "Name"
#define CAT_SEQUENCE_OID               "_id"
#define CAT_SEQUENCE_VERSION           "Version"
#define CAT_SEQUENCE_CURRENT_VALUE     "CurrentValue"
#define CAT_SEQUENCE_INCREMENT         "Increment"
#define CAT_SEQUENCE_START_VALUE       "StartValue"
#define CAT_SEQUENCE_MIN_VALUE         "MinValue"
#define CAT_SEQUENCE_MAX_VALUE         "MaxValue"
#define CAT_SEQUENCE_CACHE_SIZE        "CacheSize"
#define CAT_SEQUENCE_ACQUIRE_SIZE      "AcquireSize"
#define CAT_SEQUENCE_CYCLED            "Cycled"
#define CAT_SEQUENCE_INTERNAL          "Internal"
#define CAT_SEQUENCE_INITIAL           "Initial"
#define CAT_SEQUENCE_NEXT_VALUE        "NextValue"

#define GTS_SEQUENCE_COLLECTION_NAME         GTS_SYS_COLLECTION_SPACE_NAME".SEQUENCES"
#define GTS_SEQUENCE_NAME_INDEX              "{name:\"name_index\",key: {\""CAT_SEQUENCE_NAME"\": 1}, unique: true, enforced: true}"

#endif /* CAT_GTS_DEF_HPP_ */

