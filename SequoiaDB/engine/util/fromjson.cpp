/*******************************************************************************
   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*******************************************************************************/

#include "ossUtil.hpp"
#include "fromjson.hpp"
#include "json2rawbson.h"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"
namespace bson
{
   INT32 fromjson ( const string &str, BSONObj &out )
   {
      return fromjson ( str.c_str(), out ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_FROMJSON, "fromjson" )
   INT32 fromjson ( const CHAR *pStr, BSONObj &out )
   {
      INT32 rc         = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_FROMJSON );
      CHAR *p          = NULL ;

      // defensive code
      if ( !pStr )
      {
         // we should never hit here
         SDB_ASSERT ( FALSE, "empty str from json str" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( *pStr == '\0' )
      {
         // special case handling for empty string, we will return empty bson
         BSONObj empty ;
         out = empty ;
         // we may not need to getOwned () because BSON is using smart pointer
         // out.getOwned () ;
         goto done ;
      }
      p = json2rawbson ( pStr ) ;
      if ( p )
      {
         BSONObj::Holder *h = (BSONObj::Holder*)p ;
         try
         {
            BSONObj ret ( h ) ;
            out = ret ;
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Failed to create BSON object: %s",
                     e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         // we may not need to getOwned () because BSON is using smart pointer
         // out.getOwned () ;
      }
      else
      {
         // if json2rawbson returns NULL, that means we cannot parse json
         rc = SDB_INVALIDARG ;
      }
   done :
      PD_TRACE_EXITRC ( SDB_FROMJSON, rc );
      return rc ;
   error :
      goto done ;
   }
}

