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

   Source File Name = fapMongoMessageDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who     Description
   ====== =========== ======= ==============================================
          2020/04/21  Ting YU Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef _SDB_MONGO_MESSAGE_DEF_HPP_
#define _SDB_MONGO_MESSAGE_DEF_HPP_

namespace fap
{

const CHAR* const MONGO_DOT =                   "." ;
const CHAR* const MONGO_FIELD_NAME_CMD =        ".$cmd" ;
const CHAR* const MONGO_FIELD_NAME_DOCUMENTS =  "documents" ;
const CHAR* const MONGO_FIELD_NAME_DELETES =    "deletes" ;
const CHAR* const MONGO_FIELD_NAME_UPDATES =    "updates" ;
const CHAR* const MONGO_FIELD_NAME_INDEXES =    "indexes" ;
const CHAR* const MONGO_FIELD_NAME_INDEX =      "index" ;
const CHAR* const MONGO_FIELD_NAME_CURSORS =    "cursors" ;
const CHAR* const MONGO_FIELD_NAME_BATCHSIZE =  "batchSize" ;
const CHAR* const MONGO_FIELD_NAME_COLLECTION = "collection" ;
const CHAR* const MONGO_FIELD_VALUE_NODEJS =    "nodejs" ;


}
#endif
