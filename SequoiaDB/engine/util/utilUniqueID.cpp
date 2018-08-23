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

   Source File Name = utilUniqueID.cpp

   Descriptive Name =

   When/how to use: Process CS/CL Unique ID

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who     Description
   ====== ========== ======= ==============================================
          09/08/2018 Ting YU Initial Draft

   Last Changed =

*******************************************************************************/
#include "utilUniqueID.hpp"
#include "msgDef.h"
#include "../bson/bsonobjiterator.h"
#include "../bson/bsonobjbuilder.h"

using namespace bson ;

namespace engine
{
   // input: clInfoObj
   // [
   //    { "Name": "bar1", "UniqueID": 2667174690817 } ,
   //    { "Name": "bar2", "UniqueID": 2667174690818 }
   // ]
   //
   // output: vector<pair>
   // [
   //    < "bar1", 2667174690817 > ,
   //    < "bar2", 2667174690818 >
   // ]
   vector<PAIR_CLNAME_ID> utilBson2ClPair( const BSONObj& clInfoObj )
   {
      vector< PAIR_CLNAME_ID > clList ;

      BSONObjIterator it( clInfoObj ) ;
      while ( it.more() )
      {
         BSONElement subEle = it.next() ;
         if ( Object == subEle.type() )
         {
            BSONObj clObj = subEle.embeddedObject() ;
            BSONElement nameE = clObj.getField( FIELD_NAME_NAME ) ;
            BSONElement idE = clObj.getField( FIELD_NAME_UNIQUEID ) ;
            if ( String != nameE.type() || !idE.isNumber())
            {
               continue ;
            }
            PAIR_CLNAME_ID cl( nameE.String(), (UINT64)idE.numberLong() );
            clList.push_back( cl ) ;
         }
      }

      return clList ;
   }

   // input: clInfoObj
   // [
   //    { "Name": "bar1", "UniqueID": 2667174690817 } ,
   //    { "Name": "bar2", "UniqueID": 2667174690818 }
   // ]
   //
   // outpu: BSONObj
   // [
   //    { "Name": "bar1", "UniqueID": 0 } ,
   //    { "Name": "bar2", "UniqueID": 0 }
   // ]
   BSONObj utilUnsetUniqueID( const BSONObj& clInfoObj )
   {
      BSONArrayBuilder arrBuilder ;

      BSONObjIterator it( clInfoObj ) ;
      while ( it.more() )
      {
         BSONElement subEle = it.next() ;
         if ( Object == subEle.type() )
         {
            BSONObj clObj = subEle.embeddedObject() ;
            BSONElement nameE = clObj.getField( FIELD_NAME_NAME ) ;
            BSONElement idE = clObj.getField( FIELD_NAME_UNIQUEID ) ;
            if ( String != nameE.type() || !idE.isNumber() )
            {
               continue ;
            }
            arrBuilder << BSON( FIELD_NAME_NAME << nameE.String()
                             << FIELD_NAME_UNIQUEID << UTIL_INVALID_UNIQUEID ) ;
         }
      }

      return arrBuilder.arr() ;
   }
}
