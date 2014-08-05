#include "core.hpp"
#include "ossUtil.hpp"
#include "ossTypes.hpp"
#include "msgMessage.hpp"
#include "pd.hpp"
#include "omagent.hpp"
#include "sptUsrSsh.hpp"
#include "sptUsrCmd.hpp"
#include "sptUsrFile.hpp"
#include "sptUsrSystem.hpp"

namespace engine
{
   BOOLEAN hasLoadClass = FALSE ;
/*
   // _omaObjBuff
   _omaObjBuff::~_omaObjBuff ()
   {
      if ( _pBuff )
      {
         SDB_OSS_DEL[] _pBuff ;
         _pBuff = NULL ;
      }
      _buffLen = 0 ;
      _recordNum = 0 ;
   }

   INT32 _omaObjBuff::setObj( const CHAR *pBuff,
                                  INT32 buffLen, INT32 recordNum )
   {
      INT32 rc = SDB_OK ;
      if ( NULL == pBuff || 0 > buffLen || 0 > recordNum )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( NULL != _pBuff )
         SDB_OSS_DEL[] _pBuff ;
      _pBuff = SDB_OSS_NEW CHAR[buffLen] ;
      ossMemcpy ( _pBuff, pBuff, buffLen ) ;
      _buffLen = buffLen ;
      _recordNum = recordNum ;
   done:
     return rc ;
   error:
     goto done ;
   }
*/

   // get spider monkey engine
   _sptScope* getSptScope ()
   {
      INT32 rc = SDB_OK ;
      _sptContainer container ;
      static _sptScope *scope = container.newScope( SPT_SCOPE_TYPE_SP ) ;
      SDB_ASSERT( scope, "Failed to get spt scope" ) ;
      if ( !hasLoadClass )
      {
         rc = scope->loadUsrDefObj<_sptUsrSsh>() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
         }
         rc = scope->loadUsrDefObj<_sptUsrCmd>() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to load class _sptUsrCmd, rc = %d", rc ) ;
         }
         rc = scope->loadUsrDefObj<_sptUsrFile>() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to load class _sptUsrFile, rc = %d", rc ) ;
         }
         rc = scope->loadUsrDefObj<_sptUsrSystem>() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to load class _sptUsrSystem, rc = %d", rc ) ;
         }
         hasLoadClass = TRUE ;
      }
      return scope ;
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
