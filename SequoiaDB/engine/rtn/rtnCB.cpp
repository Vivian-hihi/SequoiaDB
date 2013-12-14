
#include "rtnCB.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "rtnContextSort.hpp"

using namespace std;
namespace engine
{

   _SDB_RTNCB::_SDB_RTNCB()
   {
      _contextHWM = 0 ;
   }

   _SDB_RTNCB::~_SDB_RTNCB()
   {
      std::map<SINT64, rtnContext *>::const_iterator it ;
      for ( it = _contextList.begin(); it != _contextList.end(); it++ )
      {
         SDB_OSS_DEL ((*it).second) ;
      }
      _contextList.clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_RTNCB_CONTEXTDEL, "_SDB_RTNCB::contextDelete" )
   void _SDB_RTNCB::contextDelete ( SINT64 contextID, pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY ( SDB__SDB_RTNCB_CONTEXTDEL ) ;

      rtnContext *pContext = NULL ;
      std::map<SINT64, rtnContext*>::iterator it ;

      if ( cb )
      {
         cb->contextDelete( contextID ) ;
      }

      {
         RTNCB_XLOCK
         it = _contextList.find( contextID ) ;
         if ( _contextList.end() != it )
         {
            pContext = it->second;
            _contextList.erase( it ) ;
         }
      }

      if ( pContext )
      {
         pContext->_release () ;
         PD_LOG( PDDEBUG, "delete context(contextID=%lld)", contextID ) ;
      }

      PD_TRACE_EXIT ( SDB__SDB_RTNCB_CONTEXTDEL ) ;
      return ;
   }

   SINT32 _SDB_RTNCB::contextNew ( RTN_CONTEXT_TYPE type,
                                   rtnContext **context,
                                   SINT64 &contextID,
                                   _pmdEDUCB * pEDUCB )
   {
      SDB_ASSERT ( context, "context pointer can't be NULL" )
      {
         RTNCB_XLOCK
         // if hit max signed 64 bit integer?
         if ( _contextHWM+1 < 0 )
         {
            return SDB_SYS ;
         }

         switch ( type )
         {
            case RTN_CONTEXT_DATA :
               (*context) = SDB_OSS_NEW rtnContextData ( _contextHWM,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_DUMP :
               (*context) = SDB_OSS_NEW rtnContextDump ( _contextHWM,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_COORD :
               (*context) = SDB_OSS_NEW rtnContextCoord ( _contextHWM,
                                                          pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_QGM :
               (*context) = SDB_OSS_NEW rtnContextQGM ( _contextHWM,
                                                        pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_TEMP :
               (*context) = SDB_OSS_NEW rtnContextTemp ( _contextHWM,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_SP :
               (*context) = SDB_OSS_NEW rtnContextSP ( _contextHWM,
                                                       pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_PARADATA :
               (*context) = SDB_OSS_NEW rtnContextParaData( _contextHWM,
                                                            pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_MAINCL :
               (*context) = SDB_OSS_NEW rtnContextMainCL( _contextHWM,
                                                         pEDUCB->getID() );
               break;
            case RTN_CONTEXT_SORT :
               (*context) = SDB_OSS_NEW rtnContextSort( _contextHWM,
                                                        pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_QGMSORT :
               (*context) = SDB_OSS_NEW rtnContextQgmSort( _contextHWM,
                                                            pEDUCB->getID() ) ;
               break ;
            default :
               PD_LOG( PDERROR, "Unknow context type: %d", type ) ;
               return SDB_SYS ;
         }

         if ( !(*context) )
         {
            return SDB_OOM ;
         }

         _contextList[_contextHWM] = *context ;
         pEDUCB->contextInsert( _contextHWM ) ;
         contextID = _contextHWM ;
         ++_contextHWM ;
      }
      PD_LOG ( PDDEBUG, "Create new context(contextID=%lld)", contextID ) ;
      return SDB_OK ;
   }

}

