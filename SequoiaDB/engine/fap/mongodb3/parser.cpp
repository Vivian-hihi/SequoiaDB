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

   Source File Name = parser.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/01/2015  LZ  Initial Draft

   Last Changed =

*******************************************************************************/
#include "parser.hpp"
#include "ossUtil.hpp"
#include "../../bson/bson.hpp"
#include "mongodef.hpp"

void msgParser::extractMsg( const CHAR *in, const INT32 inLen )
{
   const CHAR *ptr = NULL ;
   const CHAR *fullCollectionName = NULL ;
   INT32 nameLen = 0 ;

   if ( NULL != _dataStart )
   {
      _offset = 0 ;
      _currentOp = OP_INVALID ;
      _dataPacket.clear() ;
   }

   _dataStart = in ;
   _dataEnd   = in + inLen ;

   // extract header
   readInt( sizeof( INT32 ), (CHAR *)&_dataPacket.msgLen ) ;
   readInt( sizeof( INT32 ), (CHAR *)&_dataPacket.requestId ) ;
   readInt( sizeof( INT32 ), (CHAR *)&_dataPacket.responseTo ) ;
   readInt( sizeof( INT32 ), (CHAR *)&_dataPacket.opCode ) ;

   readInt( sizeof( INT32 ), (CHAR *)&_dataPacket.reservedInt ) ;

   if ( dbKillCursors == _dataPacket.opCode )
   {
      goto done ;
   }

   // extract full collection name
   fullCollectionName = _dataStart + _offset ;
   while ( *(fullCollectionName + nameLen ) )
   {
      ++nameLen ;
   }
   _offset += nameLen + 1 ;
   _dataPacket.fullName = fullCollectionName ;

   // resolve full collection name
   ptr = ossStrstr( fullCollectionName, "." ) ;
   if ( ptr )
   {
      _dataPacket.csName = _dataPacket.fullName.substr( 0,
                                                  ptr - fullCollectionName ) ;
   }
   ptr = ossStrstr( fullCollectionName, ".$cmd" ) ;
   if ( ptr )
   {
      _dataPacket.optionMask |= OPTION_CMD ;
   }

done:
   return ;
}

void msgParser::readNextObj( bson::BSONObj &obj )
{
   obj = bson::BSONObj( _nextObj ) ;
   _nextObj += obj.objsize() ;
   _offset  += obj.objsize() ;
   if ( _nextObj >= _dataEnd )
   {
      _nextObj = NULL ;
   }
}

void msgParser::readInt( const UINT32 toRead, CHAR *out )
{
   INT32 limit = toRead - 1 ;
   const CHAR *start = _dataStart + _offset ;
   if ( _bigEndian )
   {
      for ( UINT32 i = 0; i < toRead; ++i )
      {
         *( out + i ) = *( start + limit - i ) ;
      }
   }
   else
   {
      ossMemcpy( out, start, toRead ) ;
   }

   _offset += toRead ;
}
