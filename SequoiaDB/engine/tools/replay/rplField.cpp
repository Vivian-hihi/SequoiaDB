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

   Source File Name = rplField.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/04/2019  Linyoubin  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rplField.hpp"
#include "../bson/bson.hpp"
#include "ossUtil.hpp"
#include <string>

using namespace std ;
using namespace bson;
using namespace engine ;

namespace replay
{
   const CHAR RPL_FIELD_TYPE_MAPPING_STRING[]      = "MAPPING_STRING" ;
   const CHAR RPL_FIELD_TYPE_CONST_STRING[]        = "CONST_STRING" ;
   const CHAR RPL_FIELD_TYPE_OUTPUT_TIME[]         = "OUTPUT_TIME" ;
   const CHAR RPL_FIELD_TYPE_AUTO_OP[]             = "AUTO_OP";
   const CHAR RPL_FIELD_TYPE_ORIGINAL_TIME[]       = "ORIGINAL_TIME";

   rplMappingStrField::rplMappingStrField( const CHAR *sFieldName,
                                           const CHAR *tFieldName )
   {
      _hasDefaultValue = FALSE ;
      ossSnprintf( _sFieldName, MAX_FIELDNAME_LEN, "%s", sFieldName ) ;
      ossSnprintf( _tFieldName, MAX_FIELDNAME_LEN, "%s", tFieldName ) ;
   }

   rplMappingStrField::rplMappingStrField( const CHAR *sFieldName,
                                           const CHAR *tFieldName,
                                           const CHAR *defaultValue )
   {
      _hasDefaultValue = TRUE ;
      ossSnprintf( _sFieldName, MAX_FIELDNAME_LEN, "%s", sFieldName ) ;
      ossSnprintf( _tFieldName, MAX_FIELDNAME_LEN, "%s", tFieldName ) ;
      if ( NULL != defaultValue )
      {
         _defaultValue = defaultValue ;
      }
   }

   rplMappingStrField::~rplMappingStrField()
   {
   }

   EN_FieldType rplMappingStrField::getTargetType() const
   {
      return MAPPING_STRING ;
   }

   INT32 rplMappingStrField::getValue( const BSONObj &sRecord, string &value )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele = sRecord.getField( _sFieldName ) ;
      if ( ele.type() != EOO )
      {
         if ( ele.type() == String )
         {
            value = ele.str() ;
         }
         else if ( ele.type() == jstOID )
         {
            OID oid = ele.OID() ;
            value = oid.toString() ;
         }
         else
         {
            rc = SDB_INVALIDARG ;
            PD_RC_CHECK( rc, PDERROR, "Invalid field type(%s:%d), rc = %d",
                         _sFieldName, ele.type(), rc ) ;
         }
      }
      else
      {
         if ( !_hasDefaultValue )
         {
            rc = SDB_INVALIDARG ;
            PD_RC_CHECK( rc, PDERROR, "Field(%s) is not exist, rc = %d",
                         _sFieldName, rc ) ;
         }

         value = _defaultValue ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   rplConstStringField::rplConstStringField( const CHAR *constValue )
   {
      _value = constValue ;
   }

   rplConstStringField::~rplConstStringField()
   {
   }

   EN_FieldType rplConstStringField::getTargetType() const
   {
      return CONST_STRING ;
   }

   INT32 rplConstStringField::getValue( const BSONObj &sRecord, string &value )
   {
      value = _value ;
      return SDB_OK ;
   }

   rplOutputTimeField::rplOutputTimeField()
   {
   }

   rplOutputTimeField::~rplOutputTimeField()
   {
   }

   EN_FieldType rplOutputTimeField::getTargetType() const
   {
      return OUTPUT_TIME ;
   }

   void rplOutputTimeField::getCurrentTimeStr( string &timeStr )
   {
      ossTimestamp Tm ;
      CHAR szFormat[] = "%04d-%02d-%02d %02d.%02d.%02d.%06d" ;
      CHAR szTimestmpStr[ OSS_TIMESTAMP_STRING_LEN + 1 ] = { 0 } ;
      struct tm tmpTm ;

      ossGetCurrentTime( Tm ) ;
      ossLocalTime( Tm.time, tmpTm ) ;

      if ( Tm.microtm >= OSS_ONE_MILLION )
      {
         tmpTm.tm_sec ++ ;
         Tm.microtm %= OSS_ONE_MILLION ;
      }

      ossSnprintf ( szTimestmpStr, sizeof( szTimestmpStr ),
                    szFormat,
                    tmpTm.tm_year + 1900,
                    tmpTm.tm_mon + 1,
                    tmpTm.tm_mday,
                    tmpTm.tm_hour,
                    tmpTm.tm_min,
                    tmpTm.tm_sec,
                    Tm.microtm ) ;

      timeStr = szTimestmpStr ;
   }

   INT32 rplOutputTimeField::getValue( const BSONObj &sRecord, string &value )
   {
      getCurrentTimeStr( value ) ;
      return SDB_OK ;
   }

   rplOriginalTimeField::rplOriginalTimeField()
   {
   }

   rplOriginalTimeField::~rplOriginalTimeField()
   {
   }

   EN_FieldType rplOriginalTimeField::getTargetType() const
   {
      return ORIGINAL_TIME ;
   }

   INT32 rplOriginalTimeField::getValue( const BSONObj &sRecord, string &value )
   {
      // should not call this function
      return SDB_INVALIDARG ;
   }

   rplAutoOPField::rplAutoOPField()
   {
   }

   rplAutoOPField::~rplAutoOPField()
   {
   }

   EN_FieldType rplAutoOPField::getTargetType() const
   {
      return AUTO_OP ;
   }

   INT32 rplAutoOPField::getValue( const BSONObj &sRecord, string &value )
   {
      // should not call this function
      return SDB_INVALIDARG ;
   }
}

