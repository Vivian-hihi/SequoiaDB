
#include "core.hpp"
#include "netMsgHandler.hpp"
#include "catEventProcessor.hpp"
#include "msgCatalog.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include <map>

namespace engine
{

   class _SDB_DMSCB ;
   class _dpsLogWrapper ;
   class _SDB_RTNCB ;
   class _authCB ;
   class sdbCatalogueCB ;
   class _SDB_KRCB ;
   class _clsMgr ;

   class catMainController : public _netMsgHandler, public catEventProcessor
   {
   typedef std::multimap< UINT32, SINT64 > CONTEXT_LIST;
   public:
      friend INT32 pmdCatMainControllerEntryPoint(pmdEDUCB *cb, void *pData);
      catMainController(pmdEDUCB *cb) ;
      ~catMainController() ;
      INT32 handleMsg( const NET_HANDLE &handle,
                       const _MsgHeader *header,
                       const CHAR *msg ) ;
      void handleClose( const NET_HANDLE &handle, _MsgRouteID id );
      INT32 init() ;
      INT32 active() ;
      INT32 deactive() ;
   private :
      INT32 catBuildMsgEvent ( const NET_HANDLE &handle, const MsgHeader *pMsg,
                               EvntCatalogInternalEvent *&pEvent ) ;
      INT32 processGetMoreMsg ( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryDataGrp( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryCollections( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryCollectionSpaces ( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryMsg( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processKillContext(const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processAuthenticate( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processAuthCrt( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processAuthDel( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processCheckRouteID( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryDomain ( const NET_HANDLE &handle, const CHAR *pMsg ) ;

      INT32 processMsg( void *pMsg ) ;
      INT32 _ensureMetadata() ;
      INT32 _createSysIndex ( const CHAR *pCollection,
                              const CHAR *pIndex,
                              pmdEDUCB *cb ) ;
      INT32 _createSysCollection ( const CHAR *pCollection,
                                   pmdEDUCB *cb ) ;
      INT32 _processQueryRequest ( const NET_HANDLE &handle,
                                   const CHAR *pMsg,
                                   const CHAR *pCollectionName ) ;
      INT32 postMsg( const NET_HANDLE &handle, const MsgHeader *pHead );
      void addContext( const UINT32 &handle, INT64 contextID );
      void delContext( const UINT32 &handle );
      void delContext( const UINT32 &handle, INT64 contextID );
   private :
      EDUID             _nodeManagerEDUID;
      EDUID             _catalogManagerEDUID;
      _SDB_KRCB         *_pKrcb;
      pmdEDUMgr         *_pEduMgr;
      sdbCatalogueCB    *_pCatCB;
      _SDB_DMSCB        *_pDmsCB;
      _dpsLogWrapper    *_pDpsCB;
      pmdEDUCB          *_pEDUCB;
      _SDB_RTNCB        *_pRtnCB;
      _authCB           *_pAuthCB;
      pmdEDUCB          *_pNodeMgrCB;
      pmdEDUCB          *_pCataMgrCB;
      _clsMgr           *_pClsCB;
      CONTEXT_LIST      _contextLst;
   } ;
}
