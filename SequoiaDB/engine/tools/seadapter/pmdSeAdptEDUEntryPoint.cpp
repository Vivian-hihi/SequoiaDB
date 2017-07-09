#include "pmdEDUEntryPoint.hpp"

namespace engine
{
   pmdEntryPoint getEntryFuncByType ( EDU_TYPES type )
   {
      pmdEntryPoint rt = NULL ;
      static const _eduEntryInfo entry[] = {
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SEADPTMGR, TRUE,
                                pmdCBMgrEntryPoint,
                                "SeAdapter" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SE_SERVICE, TRUE,
                                pmdAsyncNetEntryPoint,
                                "SeService" ),

         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SE_INDEXR, TRUE,
                                pmdAsyncNetEntryPoint,
                                "SeIndexerReader" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SE_INDEX, FALSE,
                                pmdAsyncSessionAgentEntryPoint,
                                "SeIndexer" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SE_AGENT, FALSE,
                                pmdAsyncSessionAgentEntryPoint,
                                "SeSearcher" ),

         // For the end
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_MAXIMUM, FALSE,
                                NULL,
                                "Unknow" )
      };

      static const UINT32 number = sizeof ( entry ) / sizeof ( _eduEntryInfo ) ;

      UINT32 index = 0 ;
      for ( ; index < number ; index ++ )
      {
         if ( entry[index].type == type )
         {
            rt = entry[index].entryFunc ;
            goto done ;
         }
      }

   done :
      return rt ;
   }
}

