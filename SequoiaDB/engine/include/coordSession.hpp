#ifndef COORDSESSION_HPP__
#define COORDSESSION_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "pmdEDU.hpp"
#include "netMultiRouteAgent.hpp"

namespace engine
{
   typedef struct _subSessionInfo
   {
      MsgRouteID  routeID ;
      BOOLEAN     isConnected ;

      _subSessionInfo()
      {
         routeID.value  = MSG_INVALID_ROUTEID ;
         isConnected    = FALSE ;
      }
   } subSessionInfo ;

   typedef std::map<UINT64, subSessionInfo>     COORD_SUBSESSION_MAP ;
   typedef std::map<UINT32, MsgRouteID>         COORD_LASTNODE_MAP ;


   class CoordSession : public SDBObject
   {
   public:
      CoordSession( pmdEDUCB *pEduCB );
      ~CoordSession(){}

   public:
      INT32     addSubSession( const MsgRouteID &routeID );
      void     addSubSessionWithoutCheck( const MsgRouteID &routeID );
      BOOLEAN  delSubSession( const MsgRouteID &routeID );
      INT32    disConnect( const MsgRouteID &routeID );
      void     addLastNode( const MsgRouteID &routeID );
      MsgRouteID getLastNode( UINT32 groupID );
      void     removeLastNode( UINT32 groupID ) ;
      void     getAllSessionRoute( ROUTE_SET &routeMap );
      void     postEvent ( pmdEDUEvent const &data );
      BOOLEAN  isSubsessionConnected( const MsgRouteID &routeID );
      void     addRequest( const UINT64 reqID, const MsgRouteID &routeID );
      void     delRequest( const UINT64 reqID );
      //void     delRequest( const MsgRouteID &routeID );
      void     clearRequest();
      BOOLEAN  isValidResponse( const UINT64 reqID );
      BOOLEAN  isValidResponse( const MsgRouteID &routeID, const UINT64 reqID );

   private:
      CoordSession(){}
      CoordSession( CoordSession &coordSession ){}
      INT32 checkRemoteRouteID( const MsgRouteID &routeID );

   private:
      pmdEDUCB                   *_pEduCB;
      COORD_SUBSESSION_MAP       _subSessionMap;
      COORD_LASTNODE_MAP         _lastNodeMap;
      ossSpinXLatch              _mutex ;
      REQUESTID_MAP              _requestMap;
   } ;
}

#endif

