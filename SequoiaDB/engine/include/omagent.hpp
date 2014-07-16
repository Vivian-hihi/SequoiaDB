#ifndef OMAGENT_HPP_
#define OMAGENT_HPP_

#include "core.hpp"
#include "../bson/bson.h"
#include "ossUtil.hpp"
#include "sptApi.hpp"

using namespace engine ;
using namespace bson ;

/*
#define OMA_FIELD_NAME_PING                  "Ping"
#define OMA_FIELD_NAME_SSH                   "Ssh"
#define OMA_FIELE_NAME_HOSTNAME              "Hostname"
#define OMA_FIELD_NMAE_IP                    "Ip"
#define OMA_FIELD_NAME_SCAN_HOST_RET         "ScanHostRet"
*/

namespace CLSMGR
{
/*
   class _omagentObjBuff : public SDBObject
   {
      private:
         _omagentObjBuff( const _omagentObjBuff &right ) ;

         _omagentObjBuff& operator= ( const _omagentObjBuff &right ) ;

      public:

         _omagentObjBuff ()
         {
            _pBuff = NULL ;
            _buffSize = 0 ;
            _recordNum = 0 ;
         }

         virtual ~_omagentObjBuff () ;

         INT32 setObj ( const CHAR *pBuff, INT32 buffLen, INT32 recordNum ) ;

         const CHAR* data () { return _pBuff ; }
         INT32       size () { return _buffSize ; }
         INT32       recordNum () { return _recordNum ; }

      private:
         CHAR                 *_pBuff ;
         INT32                _buffSize ;
         INT32                _recordNum ;
   } ;

   typedef _omagentObjBuff omagentObjBuff ;
*/
   // get spider monkey engine
   _sptScope* getSptScope () ;

   // get bson field
   INT32 omagentGetIntElement ( const BSONObj &obj, const CHAR *fieldName,
                                INT32 &value ) ;

   INT32 omagentGetStringElement ( const BSONObj &obj, const CHAR *fieldName,
                                   const CHAR **value ) ;

   INT32 omagentGetObjElement ( const BSONObj &obj, const CHAR *fieldName,
                                BSONObj &value ) ;

   INT32 omagentGetBooleanElement ( const BSONObj &obj, const CHAR *fieldName,
                                    BOOLEAN &value ) ;

}





#endif // OMAGENT_HPP_
