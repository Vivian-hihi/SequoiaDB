#ifndef CATNODEMANAGER_HPP__
#define CATNODEMANAGER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "catEventProcessor.hpp"
#include "pmd.hpp"
#include "netDef.hpp"

using namespace bson ;

namespace engine
{
   class _clsMgr ;
   class sdbCatalogueCB ;
   class _SDB_RTNCB ;
   class _dpsLogWrapper ;
   class _SDB_DMSCB ;
   class _SDB_KRCB ;

   /*
      catNodeManager define
   */
   class catNodeManager : public catEventProcessor
   {
   public:
      catNodeManager() ;
      virtual ~catNodeManager() ;

      INT32 init() ;
      INT32 active() ;
      INT32 deactive() ;

      void  attachCB( pmdEDUCB *cb ) ;
      void  detachCB( pmdEDUCB *cb ) ;

   // message process functions
   protected:
      INT32 processMsg( void *pMsg ) ;
      INT32 processCommandMsg( void *pMsg, BOOLEAN writable ) ;

      INT32 processCmdCreateGrp( const CHAR *pQuery ) ;
      INT32 processCmdCreateDomain( const CHAR *pQuery ) ;
      INT32 processCmdCreateNode( const CHAR *pQuery ) ;
      INT32 processCmdUpdateNode( const CHAR *pQuery, const CHAR *pSelector ) ;
      INT32 processCmdDelNode( const CHAR *pQuery ) ;

      INT32 processGrpReq(void *pMsg);
      INT32 processRegReq(void *pMsg);
      INT32 processPrimaryChange(void *pMsg);
      INT32 processRemoveGrp(void *pMsg) ;
      INT32 processActiveGrp(void *pMsg);

      INT32 readCataConf();
      INT32 parseCatalogConf( CHAR *pData, const SINT64 sDataSize, SINT64 &sParseBytes );
      INT32 parseLine( const CHAR *pLine, BSONObj &obj );
      INT32 generateGroupInfo( bson::BSONObj &boConf, bson::BSONObj &boGroupInfo );
      INT32 saveGroupInfo ( bson::BSONObj &boGroupInfo, INT16 w );
      INT32 parseIDInfo( bson::BSONObj &obj );
      INT32 getNodeInfo( const bson::BSONObj &boReq, bson::BSONObj &boNodeInfo );
      INT32 removeGrp( const CHAR *groupName ) ;
      INT32 activeGrp( const std::string &strGroupName, bson::BSONObj &boGroupInfo );

      INT32 _count( const CHAR *collection, const BSONObj &matcher,
                    UINT64 &count ) ;

   protected:
      void  _fillRspHeader( MsgHeader *rspMsg, const MsgHeader *reqMsg ) ;
      INT32 _sendFailedRsp( NET_HANDLE handle, INT32 res, MsgHeader *reqMsg) ;

   // tool fuctions
   private:
      INT32 _createGrp( const CHAR *groupName ) ;
      INT32 _createNode( const CHAR *pQuery ) ;
      INT32 _delNode( BSONObj &boDelNodeInfo ) ;
      INT32 _addNodeToGrp ( BSONObj &boGroupInfo, BSONObj &boNodeInfo,
                            UINT16 nodeID ) ;
      INT32 _updateNodeToGrp ( BSONObj &boGroupInfo, BSONObj &boNodeInfoNew,
                               UINT16 nodeID ) ;
      INT32 _getRemovedGroupsObj( const BSONObj &srcGroupsObj,
                                  UINT16 removeNode,
                                  BSONArrayBuilder &newObjBuilder ) ;
      INT32 _getRemovedGroupsObj( const BSONObj &srcGroupsObj,
                                  const CHAR *hostName,
                                  const CHAR *serviceName,
                                  BSONArrayBuilder &newObjBuilder,
                                  INT32 *pRemoveNodeID = NULL ) ;
      INT32 _checkNodeInfo( BSONObj &boNodeInfo, INT32 nodeRole,
                            BSONObjBuilder *newObjBuilder = NULL ) ;
      string _getServiceName( UINT16 localPort, _MSG_ROUTE_SERVICE_TYPE type ) ;
      INT16 _majoritySize() ;

      INT32 _getNodeInfoByConf( BSONObj &boConf, BSONObjBuilder &bobNodeInfo ) ;

   private:
      typedef enum _SDB_CAT_MODULE_STATUS
      {
         SDB_CAT_MODULE_ACTIVE    =  0,
         SDB_CAT_MODULE_DEACTIVE
      }SDB_CAT_MODULE_STATUS;

   private:
      SDB_CAT_MODULE_STATUS      _status;
      _SDB_KRCB                  *_pKrcb;
      _SDB_DMSCB                 *_pDmsCB;
      _dpsLogWrapper             *_pDpsCB;
      _SDB_RTNCB                 *_pRtnCB;
      pmdEDUMgr                  *_pEduMgr;
      sdbCatalogueCB             *_pCatCB;
      pmdEDUCB                   *_pEduCB;
      _clsMgr                    *_pClsCB;
   };
}

#endif
