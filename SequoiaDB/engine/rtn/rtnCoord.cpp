#include "rtnCoord.hpp"
#include "rtnCoordCommands.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

namespace engine
{

   rtnCoordProcesserFactory::rtnCoordProcesserFactory()
   {
      addCommand();
      addOperator();
   }
   
   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOPROFAC_RTNCOPROFAC, "rtnCoordProcesserFactory::~rtnCoordProcesserFactory" )
   rtnCoordProcesserFactory::~rtnCoordProcesserFactory()
   {
      PD_TRACE_ENTRY ( SDB_RTNCOPROFAC_RTNCOPROFAC ) ;
      COORD_CMD_MAP::iterator iter;
      iter = _cmdMap.begin();
      while ( iter != _cmdMap.end() )
      {
         SDB_OSS_DEL iter->second;
         _cmdMap.erase( iter++ );
      }
      _cmdMap.clear();

      COORD_OP_MAP::iterator iterOp;
      iterOp = _opMap.begin();
      while( iterOp != _opMap.end() )
      {
         SDB_OSS_DEL iterOp->second;
         _opMap.erase( iterOp++ );
      }
      PD_TRACE_EXIT ( SDB_RTNCOPROFAC_RTNCOPROFAC ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOPROFAC_GETOP, "rtnCoordProcesserFactory::getOperator" )
   rtnCoordOperator * rtnCoordProcesserFactory::getOperator( SINT32 opCode )
   {
      PD_TRACE_ENTRY ( SDB_RTNCOPROFAC_GETOP ) ;
      COORD_OP_MAP::iterator iter;
      iter = _opMap.find ( opCode );
      if ( _opMap.end() == iter )
      {
         iter = _opMap.find ( MSG_NULL );
      }
      PD_TRACE_EXIT ( SDB_RTNCOPROFAC_GETOP ) ;
      return iter->second;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOPROFAC_GETCOMPRO1, "rtnCoordProcesserFactory::getCommandProcesser" )
   rtnCoordCommand * rtnCoordProcesserFactory::getCommandProcesser(const MsgOpQuery *pQuery)
   {
      PD_TRACE_ENTRY ( SDB_RTNCOPROFAC_GETCOMPRO1 ) ;
      SDB_ASSERT ( pQuery, "pQuery can't be NULL" )
      rtnCoordCommand *pProcesser = NULL;
      do
      {
         if ( MSG_BS_QUERY_REQ == pQuery->header.opCode )
         {
            if ( pQuery->nameLength > 0 )
            {
               COORD_CMD_MAP::iterator iter;
               iter = _cmdMap.find( pQuery->name );
               if ( iter != _cmdMap.end() )
               {
                  pProcesser = iter->second;
               }
            }
         }
         if ( NULL == pProcesser )
         {
            COORD_CMD_MAP::iterator iter;
            iter = _cmdMap.find( COORD_CMD_DEFAULT );
            if ( iter != _cmdMap.end() )
            {
               pProcesser = iter->second;
            }
         }
      }while ( FALSE );
      PD_TRACE_EXIT ( SDB_RTNCOPROFAC_GETCOMPRO1 ) ;
      return pProcesser;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOPROFAC_GETCOMPRO2, "rtnCoordProcesserFactory::getCommandProcesser" )
   rtnCoordCommand * rtnCoordProcesserFactory::getCommandProcesser(const char *pCmd)
   {
      PD_TRACE_ENTRY ( SDB_RTNCOPROFAC_GETCOMPRO2 ) ;
      SDB_ASSERT ( pCmd, "pCmd can't be NULL" )
      rtnCoordCommand *pProcesser = NULL;
      do
      {
         COORD_CMD_MAP::iterator iter;
         iter = _cmdMap.find( pCmd );
         if ( iter != _cmdMap.end() )
         {
            pProcesser = iter->second;
         }
      }while ( FALSE );
      PD_TRACE_EXIT ( SDB_RTNCOPROFAC_GETCOMPRO2 ) ;
      return pProcesser;
   }
}
