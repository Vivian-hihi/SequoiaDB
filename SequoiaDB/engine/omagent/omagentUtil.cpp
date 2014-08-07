/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = omagentUtil.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "ossPrimitiveFileOp.hpp"
#include "pd.hpp"
#include "omagentUtil.hpp"
#include "spt.hpp"
#include "sptSPScope.hpp"
#include "sptUsrSsh.hpp"
#include "sptUsrCmd.hpp"
#include "sptUsrFile.hpp"
#include "sptUsrSystem.hpp"
#include "../spt/js_in_cpp.hpp"


JSBool InitDbClasses( JSContext *cx, JSObject *obj ) ;

namespace engine
{

   INT32 checkBuffer ( CHAR **ppBuffer, INT32 *bufferSize,
                       INT32 packetLength )
   {
      INT32 rc = SDB_OK ;
      if ( packetLength > *bufferSize )
      {
         CHAR *pOrigMem = *ppBuffer ;
         INT32 newSize = ossRoundUpToMultipleX ( packetLength, SDB_PAGE_SIZE ) ;
         if ( newSize < 0 )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "new buffer overflow" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         *ppBuffer = (CHAR*)SDB_OSS_REALLOC ( *ppBuffer, sizeof(CHAR)*(newSize)) ;
         if ( !*ppBuffer )
         {
            PD_LOG ( PDERROR, "Failed to allocate %d bytes send buffer",
                     newSize ) ;
            rc = SDB_OOM ;
            // realloc does NOT free original memory if it fails, so we have to
            // assign pointer to original
            *ppBuffer = pOrigMem ;
            goto error ;
         }
         *bufferSize = newSize ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }


   INT32 readFile ( const CHAR * name , CHAR ** buf , UINT32 * bufSize,
                    UINT32 * readSize )
   {
      ossPrimitiveFileOp op ;
      ossPrimitiveFileOp::offsetType offset ;
      INT32 rc = SDB_OK ;

      SDB_ASSERT ( name && buf && bufSize, "Invalid arguments" ) ;

      rc = op.Open ( name , OSS_PRIMITIVE_FILE_OP_READ_WRITE ) ;
      if ( rc != SDB_OK )
      {
         ossPrintf ( "Can't open file: %s"OSS_NEWLINE, name ) ;
         goto error ;
      }

      rc = op.getSize ( &offset ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }

      if ( *bufSize < offset.offset + 1 )
      {
         if ( *buf )
         {
            SDB_OSS_FREE( *buf ) ;
            *buf = NULL ;
            *bufSize = 0 ;
         }
         *buf = (CHAR *) SDB_OSS_MALLOC ( offset.offset + 1 ) ;
         if ( ! *buf )
         {
            rc = SDB_OOM ;
            PD_LOG ( PDERROR , "fail to alloc memory" ) ;
            goto error ;
         }
         *bufSize = offset.offset + 1 ;
      }

      rc = op.Read ( offset.offset , *buf , NULL ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }
      (*buf)[ offset.offset ] = 0 ;
      if ( readSize ) *readSize = offset.offset ;

   done :
      op.Close() ;
      return rc ;
   error :
      goto done ;
   }

   // get spider monkey engine
   INT32 getSptScope ( _sptScope **scope )
   {
      INT32 rc = SDB_OK ;
      _sptContainer container ;
      _sptScope *_scope = container.newScope( SPT_SCOPE_TYPE_SP ) ;
      SDB_ASSERT( _scope, "Failed to get spt scope" ) ;
      // init db classes for omagent to use db driver API
      if ( !InitDbClasses( ((sptSPScope *)_scope)->getContext(),
                           ((sptSPScope *)_scope)->getGlobalObj() ) )
      {
         PD_LOG( PDERROR, "failed to init dbclass" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      // init another classes for omagent to get hosts info and so on
      rc = _scope->loadUsrDefObj<_sptUsrSsh>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = _scope->loadUsrDefObj<_sptUsrCmd>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = _scope->loadUsrDefObj<_sptUsrFile>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = _scope->loadUsrDefObj<_sptUsrSystem>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = evalInitScripts2( _scope ) ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to init spt scope, rc = %d", rc ) ;
      // return result
      *scope = _scope ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // get bson field
   INT32 omaGetIntElement ( const BSONObj &obj, const CHAR *fieldName,
                                INT32 &value )
   {
      SINT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName, "field name can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( ele.isNumber(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be Integer",
                 obj.toString().c_str()) ;
      value = ele.numberInt() ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omaGetStringElement ( const BSONObj &obj, const CHAR *fieldName,
                                   const CHAR **value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName && value, "field name and value can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( String == ele.type(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be String",
                 obj.toString().c_str()) ;
      *value = ele.valuestr() ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omaGetObjElement ( const BSONObj &obj, const CHAR *fieldName,
                                BSONObj &value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName , "field name can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( Object == ele.type(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be Object",
                 obj.toString().c_str()) ;
      value = ele.embeddedObject() ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omaGetBooleanElement ( const BSONObj &obj, const CHAR *fieldName,
                                    BOOLEAN &value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName , "field name can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( Bool == ele.type(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be Bool",
                 obj.toString().c_str()) ;
      value = ele.boolean() ;
   done :
      return rc ;
   error :
      goto done ;
   }

}
