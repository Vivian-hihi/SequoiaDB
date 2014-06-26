#ifndef OMAGENT_HPP_
#define OMAGENT_HPP_

#include "core.hpp"

namespace CLSMGR
{

   class _omagentObjBuff : public SDBObject
   {
      private:
         _omagentObjBuff( const _omagentObjBuf &right ) ;

         _omagentObjBuff& operator= ( const _omagentObjBuf &right ) ;

      public:

         _omagentObjBuff ()
         {
            _pBuff = NULL ;
            _buffLen = 0 ;
            _recordNum = 0 ;
         }

         virtual ~_omagentObjBuff () ;

         INT32 setObj ( const CHAR *pBuff, INT32 buffLen, INT32 recordNum ) ;

         const CHAR* data () { return _pBuff ; }
         INT32       size () { return _buffSize ; }
         INT32       recordNum ()( return _recordNum ; }

      private:
         CHAR                *_pBuff ;
         INT32                _buffSize ;
         INT32                _recordNum ;
   } ;

   typedef _omagentObjBuff omagentObjBuff ;

/*
   omagent control func
*/

   BOOLEAN omagentIsCommand ( const CHAR *name ) ;

   INT32 omagentParseCommand ( const CHAR *name,
                               _omagentCommand **ppCommand ) ;

   INT32 omagentInitCommand ( _omagentCommand *pCommand ,INT32 flags,
                              INT64 numToSkip,
                              INT64 numToReturn, const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff, const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

   INT32 omagentRunCommand ( _omagentCommand *pCommand, omagentObjBuff &objBuff ) ;

   INT32 omagentReleaseCommand ( _omagentCommand **ppCommand ) ;

}





#endif // OMAGENT_HPP_
