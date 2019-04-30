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

   Source File Name = rplFieldInfo.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/04/2019  Linyoubin  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef REPLAY_FIELD_INFO_HPP_
#define REPLAY_FIELD_INFO_HPP_

#include "oss.hpp"
#include "rplOutputter.hpp"
#include <string>
#include <map>

using namespace std ;
using namespace engine ;

namespace replay
{
   enum EN_FieldType
   {
      MAPPING_STRING = 0,       // mapping string

      CONST_STRING = 100, // const string value
      OUTPUT_TIME = 101,    // output time
      AUTO_OP = 102,    //auto generate op str
      ORIGINAL_TIME = 200  // replica log's original time
   } ;

   extern const CHAR RPL_FIELD_TYPE_MAPPING_STRING[] ; // mapping string
   extern const CHAR RPL_FIELD_TYPE_CONST_STRING[] ;   // const string value
   extern const CHAR RPL_FIELD_TYPE_OUTPUT_TIME[] ;   // output time
   extern const CHAR RPL_FIELD_TYPE_AUTO_OP[] ;       //auto generate op str
   extern const CHAR RPL_FIELD_TYPE_ORIGINAL_TIME[] ; // replica log's original time

   class rplField
   {
   public:
      virtual ~rplField() {} ;

   public:
      virtual EN_FieldType getTargetType() const = 0 ;
      virtual INT32 getValue( const BSONObj &sRecord, string &value ) = 0 ;
   } ;

   const INT32 MAX_FIELDNAME_LEN = 1024 ;

   class rplMappingStrField : public rplField, public SDBObject
   {
   public:
      rplMappingStrField( const CHAR *sFieldName, const CHAR *tFieldName ) ;
      rplMappingStrField( const CHAR *sFieldName, const CHAR *tFieldName,
                          const CHAR *defaultValue ) ;
      ~rplMappingStrField() ;

   public:
      EN_FieldType getTargetType() const ;
      INT32 getValue( const BSONObj &sRecord, string &value ) ;

   private:
      BOOLEAN _hasDefaultValue ;
      string _defaultValue ;
      CHAR _sFieldName[ MAX_FIELDNAME_LEN + 1 ] ;
      CHAR _tFieldName[ MAX_FIELDNAME_LEN + 1 ] ;
   } ;

   class rplConstStringField : public rplField, public SDBObject
   {
   public:
      rplConstStringField( const CHAR *constValue ) ;
      ~rplConstStringField() ;

   public:
      EN_FieldType getTargetType() const ;
      INT32 getValue( const BSONObj &sRecord, string &value ) ;

   private:
      string _value ;
   } ;

   class rplOutputTimeField : public rplField, public SDBObject
   {
   public:
      rplOutputTimeField() ;
      ~rplOutputTimeField() ;

   public:
      EN_FieldType getTargetType() const ;
      INT32 getValue( const BSONObj &sRecord, string &value ) ;

   private:
      void getCurrentTimeStr( string &timeStr ) ;
   } ;

   class rplOriginalTimeField : public rplField, public SDBObject
   {
   public:
      rplOriginalTimeField() ;
      ~rplOriginalTimeField() ;

   public:
      EN_FieldType getTargetType() const ;
      INT32 getValue( const BSONObj &sRecord, string &value ) ;

   public:
      static void getTimeStr( const UINT64 &microSeconds, CHAR *szTimeStr,
                              INT32 timeStrSize ) ;
   } ;

   class rplAutoOPField : public rplField, public SDBObject
   {
   public:
      rplAutoOPField() ;
      ~rplAutoOPField() ;

   public:
      EN_FieldType getTargetType() const ;
      INT32 getValue( const BSONObj &sRecord, string &value ) ;
   } ;
}

#endif // REPLAY_FIELD_INFO_HPP_

