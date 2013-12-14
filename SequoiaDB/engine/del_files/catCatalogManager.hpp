#ifndef CATCATALOGUEMANAGER_HPP_
#define CATCATALOGUEMANAGER_HPP_

#include "core.hpp"
#include "catEventProcessor.hpp"

namespace engine
{
   class catCatalogueManager : public catEventProcessor
   {
   public:
      catCatalogueManager(pmdEDUCB *cb);
      INT32 init();
   private:
      INT32 _buildInitBound ( INT32 groupID,
                              INT32 numFields,
                              BSONObj &outputObj ) ;
      INT32 processMsg( void *pMsg );
      INT32 active();
      INT32 deactive();
      INT32 processCreateCollectionSpace( void *pMsg );
      INT32 processCreateCollection ( void *pMsg );
      INT32 processQueryCatalogue ( void *pMsg );
      INT32 processDropCollection ( void *pMsg );
      INT32 processDropCollectionSpace ( void *pMsg );
      INT32 processQuerySpaceInfo ( void *pMsg );

      INT32 assignGroups( bson::BSONObj &groupInfoArr, bson::BSONObj &groupIdArr  );
      INT32 getGroupInfo( UINT32 groupID, bson::BSONObj &obj );
      INT32 resolveCollectionName ( const CHAR *pInput, UINT32 inputLen,
                                 CHAR *pSpaceName, UINT32 spaceNameSize,
                                 CHAR *pCollectionName, UINT32 collectionNameSize );

      INT32 checkIfCollectionExist( const char *pCollectionName,  // include spacename: space.collection
                                 BOOLEAN &isExist,
                                 bson::BSONObj &obj );
      INT32 delCollectionSpace ( const std::string &strSpaceName );

      INT32 checkIfSpaceExist( const char *pSpaceName, 
                                 BOOLEAN &isExist,
                                 bson::BSONObj &obj );
      INT32 buildCatalogRecord( const CHAR *pCollectonName,
                              bson::BSONElement &beGroupID,
                              bson::BSONElement &beShardingKey,
                              bson::BSONObj &catRecord );

   private:
      pmdKRCB              *_pKrcb;
      sdbCatalogueCB       *_pCatCB;
      SDB_DMSCB            *_pDmsCB;
      SDB_DPSCB            *_pDpsCB;
      SDB_RTNCB            *_pRtnCB;
      pmdEDUCB             *_pEduCB;
      clsCB                *_pClsCB;
   };
}

#endif
