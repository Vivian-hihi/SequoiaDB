#ifndef UTIL_ESCLT_HPP_
#define UTIL_ESCLT_HPP_

#include <string>
#include <sstream>
#include <list>
#include <vector>

#include "oss.hpp"
#include "utilHttp.hpp"
#include "cJSON.h"
#include "../bson/bsonobj.h"
#include "../util/fromjson.hpp"
#include "utilESClt.hpp"
#include "utilCommObjBuff.hpp"

#define UTIL_SE_MAX_URL_SIZE              2048
#define UTIL_ES_DFT_SCROLL_SIZE           1000
#define UTIL_SE_MAX_TYPE_SZ               255
using std::string ;

namespace engine
{
   // Client class for ElasticSearch.
   class _utilESClt : public SDBObject
   {
      public:
         _utilESClt();
         ~_utilESClt();

         // Init connection with specified uri.
         INT32 init( const string &uri, BOOLEAN readOnly = FALSE ) ;
         BOOLEAN isActive() ;
         INT32 getSEInfo( BSONObj &infoObj ) ;
         INT32 indexExist( const CHAR *index, BOOLEAN &exist ) ;
         INT32 createIndex( const CHAR *index, const CHAR *data = NULL ) ;
         INT32 dropIndex( const CHAR *index ) ;
         INT32 indexDocument( const CHAR *index, const CHAR *type,
                              const CHAR *id, const CHAR *jsonData ) ;
         INT32 updateDocument( const CHAR *index, const CHAR *type,
                               const CHAR *id, const CHAR *newData ) ;
         // INT32 upsertDocument( const CHAR *index, const CHAR *type,
         //                       const CHAR *id, const CHAR *newData ) ;
         INT32 deleteDocument( const CHAR *index, const CHAR *type,
                               const CHAR *id ) ;

         // TODO: Whether to filter meta data away. Provide another parameter?
         // Request the document by index/type/id. At most one document should be
         // returned as id dose not duplicate.
         INT32 getDocument( const CHAR *index, const CHAR *type, const CHAR *id,
                            BSONObj &result, BOOLEAN withMeta = TRUE ) ;

         // Request documents by index, type, and a K/V pair as query condition.
         INT32 getDocument( const CHAR *index, const CHAR *type,
                            const CHAR *key, const CHAR *value,
                            utilCommObjBuff &objBuff, BOOLEAN withMeta = TRUE ) ;

         // Request documents by index, type, and a query string.
         INT32 getDocument( const CHAR *index, const CHAR *type,
                            const CHAR *query, utilCommObjBuff &objBuff,
                            BOOLEAN withMeta = TRUE ) ;

         INT32 documentExist( const CHAR *index, const CHAR *type,
                              const CHAR *id, BOOLEAN &exist ) ;
         INT32 documentExist( const CHAR *index, const CHAR *type,
                              const CHAR *key, const CHAR *val,
                              BOOLEAN &exist ) ;
         INT32 deleteAllByType( const CHAR *index, const CHAR *type ) ;
         INT32 getDocCount( const CHAR *index, const CHAR *type,
                            UINT64 &count ) ;
         // To make all documents searchable now. Normally they are searchable 1s
         // after insertion.
         INT32 refresh( const CHAR *index ) ;

         INT32 initScroll( string& scrollId,
                           const CHAR* index,
                           const CHAR* type,
                           const string& query,
                           utilCommObjBuff &result,
                           int scrollSize = UTIL_ES_DFT_SCROLL_SIZE ) ;
         INT32 scrollNext( string& scrollId, utilCommObjBuff &result ) ;
         void clearScroll( const string& scrollId ) ;

      private:
         OSS_INLINE INT32 _processReply( INT32 returnCode, const CHAR *reply,
                                         INT32 replyLen, BSONObj &resultObj,
                                         BOOLEAN transform = TRUE ) ;
         INT32 _getResultObjs( const BSONObj &replyObj,
                               utilCommObjBuff &resultObjs ) ;

         INT32 _searchByUri( const CHAR *index, const CHAR *type,
                             const string &query,
                             string &result, UINT64 *totalNum ) ;
         INT32 _searchByDSL( const CHAR *index, const CHAR *type,
                             const string &query,
                             const CHAR **ppReply, INT32 *replyLen ) ;
         void _getHitNum( const BSONObj &fullResult, INT64 &hitNum ) ;


      private:
         utilHttp _http;
         BOOLEAN _readOnly;
   };
   typedef _utilESClt utilESClt ;

   // Process the return information of http request. If the original return
   // code is not SDB_OK, return the original return code.
   // Otherwise, convert the result from json string to BSONObj.
   OSS_INLINE INT32 _utilESClt::_processReply( INT32 returnCode,
                                               const CHAR *reply,
                                               INT32 replyLen,
                                               BSONObj &resultObj,
                                               BOOLEAN transform )
   {
      INT32 rc = SDB_OK ;

      if ( SDB_OK == returnCode )
      {
         // Request process successfully. Let's get the result in BSONObj format.
         if ( transform && reply && replyLen > 0 )
         {
            rc = fromjson( reply, resultObj ) ;
            PD_RC_CHECK( rc, PDERROR, "Convert respond to BSONObj "
                         "failed[ %d ], respond: %s",
                         rc, string( reply, replyLen ).c_str() ) ;
         }
      }
      else
      {
         rc = returnCode ;
         if ( reply && replyLen > 0 )
         {
            PD_LOG( PDERROR, "Request processed failed[ %d ], respond "
                    "message: %s", rc, string( reply, replyLen ).c_str() ) ;
         }
         else
         {
            PD_LOG( PDERROR, "Request processed failed[ %d ]", rc ) ;
         }

         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }
}

#endif /* UTIL_ESCLT_HPP_ */

