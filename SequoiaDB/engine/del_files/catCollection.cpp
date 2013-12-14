#include "catCollection.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
namespace engine
{
   INT32 _catCollection::resync ( BOOLEAN isInitial )
   {
      INT32 rc = SDB_OK ;
      return rc ;
   }
   INT32 _catCollection::findReplicationGroups ( CHAR *pCollectionName,
         BSONObj &matcher, vector<CAT_REPLGROUP_ID> &replGroups )
   {
      INT32 rc = SDB_OK ;
      return rc ;
   }

   // this is called only in CATALOG mode
   INT32 _catCollection::init ()
   {
      INT32 rc                   = SDB_OK ;
      pmdKRCB *krcb              = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB           = krcb->getDMSCB() ;
      BOOLEAN retried            = FALSE ;
      SDB_ASSERT ( SDB_ROLE_CATALOG == krcb->getDBRole(),
                   "Database role must be catalog" )
   retry:
      // then let's try to find the collection
      rc = rtnFindCollection ( CAT_COLLECTION_CATALOG_COLLECTION, dmsCB ) ;
      // if we can't find the collection
      if ( rc )
      {
         // let's see if we have tried to create new collection
         if ( retried )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Cannot find collection %s",
                    CAT_COLLECTION_CATALOG_COLLECTION ) ;
            goto error ;
         }
         // if not tried, let's create one
         rc = rtnCreateCollectionCommand (
               CAT_COLLECTION_CATALOG_COLLECTION,
               NULL,
               dmsCB, NULL ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to create %s collection, rc = %d",
                    CAT_COLLECTION_CATALOG_COLLECTION, rc ) ;
            goto error ;
         }
         // mark we have tried
         retried = TRUE ;
         goto retry ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
}
