#ifndef CATCATALOGUEMANAGER_HPP_
#define CATCATALOGUEMANAGER_HPP_

#include "core.hpp"
#include "catEventProcessor.hpp"
#include "pmd.hpp"
#include "catSplit.hpp"

using namespace bson ;

namespace engine
{

   class _clsMgr ;
   class _SDB_RTNCB ;
   class _dpsLogWrapper ;
   class sdbCatalogueCB ;
   class _SDB_KRCB ;
   class _SDB_DMSCB ;

   #define CAT_MASK_CLNAME          0x00000001
   #define CAT_MASK_SHDKEY          0x00000002
   #define CAT_MASK_REPLSIZE        0x00000004
   #define CAT_MASK_SHDIDX          0x00000008
   #define CAT_MASK_SHDTYPE         0x00000010
   #define CAT_MASK_SHDPARTITION    0x00000020
   #define CAT_MASK_COMPRESSED      0x00000040
   #define CAT_MASK_ISMAINCL        0x00000080

   struct _catCollectionInfo
   {
      const CHAR  *_pCLName ;
      BSONObj     _shardingKey ;
      INT32       _replSize ;
      bool        _enSureShardIndex ;
      const CHAR  *_pShardingType ;
      INT32       _shardPartition ;
      BOOLEAN     _isHash ;
      BOOLEAN     _isSharding ;
      BOOLEAN     _isCompressed ;
      bool        _isMainCL;
      std::vector<std::string>   _subCLList;

      _catCollectionInfo()
      {
         _pCLName             = NULL ;
         _replSize            = 1 ;
         _enSureShardIndex    = true ;
         _pShardingType       = CAT_SHARDING_TYPE_RANGE ;
         _shardPartition      = CAT_SHARDING_PARTITION_DEFAULT ;
         _isHash              = FALSE ;
         _isSharding          = FALSE ;
         _isCompressed        = FALSE ;
         _isMainCL            = false ;
      }
   };
   typedef _catCollectionInfo catCollectionInfo ;

   struct _catCSInfo
   {
      const CHAR  *_pCSName ;
      INT32       _pageSize ;
      const CHAR  *_domainName ;

      _catCSInfo()
      {
         _pCSName = NULL ;
         _pageSize = DMS_PAGE_SIZE_DFT ;
         _domainName = NULL ;
      }
   } ;
   typedef _catCSInfo catCSInfo ;

   class catCatalogueManager : public catEventProcessor
   {
   public:
      catCatalogueManager(pmdEDUCB *cb);
      INT32 init();

   protected:
      INT32 active() ;
      INT32 deactive() ;

   // message process functions
   protected:
      INT32 processMsg( void *pMsg ) ;
      INT32 processCommandMsg( void *pMsg, BOOLEAN writable ) ;

      INT32 processCmdCreateCL( const CHAR *pQuery,
                                const CHAR *pSelector,
                                CHAR **ppReplyBody,
                                UINT32 &replyBodyLen,
                                INT32 &returnNum ) ;
      INT32 processCmdCreateCS( const CHAR *pQuery,
                                const CHAR *pSelector,
                                CHAR **ppReplyBody,
                                UINT32 &replyBodyLen,
                                INT32 &returnNum ) ;
      INT32 processCmdSplit( const CHAR *pQuery,
                             INT32 opCode,
                             CHAR **ppReplyBody,
                             UINT32 &replyBodyLen,
                             INT32 &returnNum,
                             BOOLEAN &fillPeerRouteID ) ;
      INT32 processCmdQuerySpaceInfo( const CHAR *pQuery,
                                      CHAR **ppReplyBody,
                                      UINT32 &replyBodyLen,
                                      INT32 &returnNum ) ;
      INT32 processCmdDropCollection ( const CHAR *pQuery ) ;
      INT32 processCmdDropCollectionSpace ( const CHAR *pQuery ) ;

      INT32 processQueryCatalogue ( void *pMsg );
      INT32 processQueryTask ( void *pMsg ) ;
      INT32 processAlterCollection ( void *pMsg ) ;
      INT32 processCmdCrtProcedures( void *pMsg ) ;
      INT32 processCmdRmProcedures( void *pMsg ) ;
      INT32 processCmdLinkCollection( const CHAR *pQuery,
                                    CHAR **ppReplyBody,
                                    UINT32 &replyBodyLen,
                                    INT32 &returnNum );
      INT32 processCmdUnlinkCollection( const CHAR *pQuery,
                                       CHAR **ppReplyBody,
                                       UINT32 &replyBodyLen,
                                       INT32 &returnNum );

   // tool functions
   protected:
      void  _fillRspHeader( MsgHeader *rspMsg, const MsgHeader *reqMsg ) ;
      INT32 _sendFailedRsp( NET_HANDLE handle, INT32 res, MsgHeader *reqMsg) ;

      INT32 _createCL( BSONObj & createObj, BSONObj & selector,
                       INT32 &groupID ) ;
      INT32 _createCS( BSONObj & createObj, BSONObj & selector,
                       INT32 &groupID ) ;
      INT32 _checkAndBuildCataRecord( const BSONObj &infoObj,
                                      UINT32 &fieldMask,
                                      catCollectionInfo &clInfo ) ;
      INT32 _checkCSObj( const BSONObj &infoObj,
                         catCSInfo &csInfo ) ;

      INT32 _checkGroupInDomain( const CHAR *groupName,
                                 const CHAR *domainName,
                                 BOOLEAN &existed,
                                 INT32 *pGroupID = NULL ) ;
      INT32 _assignGroup( vector< INT32 > *pGoups, INT32 &groupID ) ;

      INT32 _buildCatalogRecord( const catCollectionInfo &clInfo,
                                 INT32 groupID,
                                 const CHAR *groupName,
                                 BSONObj &catRecord ) ;

   private:
      INT32 _buildInitBound ( UINT32 fieldNum,
                              const Ordering& order,
                              BSONObj& lowBound,
                              BSONObj& upBound ) ;

      INT32 _buildHashBound( BSONObj& lowBound,
                             BSONObj& upBound,
                             INT32 paritition ) ;

      INT16 _majoritySize() ;

   private:
      _SDB_KRCB            *_pKrcb;
      sdbCatalogueCB       *_pCatCB;
      _SDB_DMSCB           *_pDmsCB;
      _dpsLogWrapper       *_pDpsCB;
      _SDB_RTNCB           *_pRtnCB;
      pmdEDUCB             *_pEduCB;
      _clsMgr              *_pClsCB;
      clsTaskMgr           _taskMgr ;

   };
}

#endif
