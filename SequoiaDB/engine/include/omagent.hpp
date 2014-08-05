#ifndef OMAGENT_HPP_
#define OMAGENT_HPP_

#include "core.hpp"
#include "../bson/bson.h"
#include "ossUtil.hpp"
#include "sptApi.hpp"
#include "omagentMsgDef.hpp"

using namespace bson ;

#define JS_ARG_LEN 512

namespace engine
{
/*
   struct _confFields
   {
      CHAR *_hostName ;
      CHAR *_dataGroupName ;
      CHAR *_dbPath ;
      CHAR *_svcName ;
      CHAR *_diagLevel ;
      CHAR *_role ;
      CHAR *_logFileSize ;
      CHAR *_logFileNum ;
      CHAR *_transactionOn ;
      CHAR *_prefInst ;
      CHAR *_numPageCleaners ;
      CHAR *_pageCleanInterval ;
   } ;
   typedef struct _confFields confFields ;
*/

   struct _InstallInfo
   {
      const CHAR *_hostName ;
      const CHAR *_svcName ;
      const CHAR *_dbPath ;
      const CHAR *_confPath ;
      const CHAR *_dataGroupName ;
      BSONObj _conf ;
   } ;
   typedef struct _InstallInfo InstallInfo ;

   struct _InstallJobResult
   {
      INT32 _rc ;
      std::string _errMsg ;
      INT32 _totalNum ;
      INT32 _finishNum ;
      std::vector< InstallInfo > _finishNode ;
   } ;
   typedef struct _InstallJobResult InstallJobResult ;

   // get spider monkey engine
   _sptScope* getSptScope () ;

   // get bson field
   INT32 omaGetIntElement ( const BSONObj &obj, const CHAR *fieldName,
                                INT32 &value ) ;

   INT32 omaGetStringElement ( const BSONObj &obj, const CHAR *fieldName,
                                   const CHAR **value ) ;

   INT32 omaGetObjElement ( const BSONObj &obj, const CHAR *fieldName,
                                BSONObj &value ) ;

   INT32 omaGetBooleanElement ( const BSONObj &obj, const CHAR *fieldName,
                                    BOOLEAN &value ) ;

}





#endif // OMAGENT_HPP_
