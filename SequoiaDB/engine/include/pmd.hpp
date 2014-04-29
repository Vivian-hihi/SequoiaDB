/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = pmd.hpp

   Descriptive Name = Process MoDel Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure kernel control block,
   which is the most critical data structure in the engine process.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMD_HPP__
#define PMD_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "ossUtil.hpp"
#include "pmdEDUMgr.hpp"
#include "pd.hpp"
#include "monCB.hpp"
#include "pmdOptionsMgr.hpp"
#include "msg.h"
#include "msgDef.hpp"
#include "pmdEnv.hpp"
#include "sdbInterface.hpp"

class _pdTraceCB ;

namespace engine
{

   /*
      PMD DB status define
   */
   enum PMD_DB_STATUS
   {
      PMD_DB_NORMAL        = 0 ,
      PMD_DB_SHUTDOWN      = 1
   } ;

   #define PMD_IS_DB_NORMAL   ( PMD_DB_NORMAL == pmdGetKRCB()->getDBStatus() )
   #define PMD_IS_DB_DOWN     ( !PMD_IS_DB_UP  )
   #define PMD_IS_DB_UP       PMD_IS_DB_NORMAL

   #define PMD_SHUTDOWN_DB(code)  \
      do { \
         pmdGetKRCB()->setDBStatus( PMD_DB_SHUTDOWN ) ; \
         pmdGetKRCB()->setExitCode( code ) ; \
      } while ( 0 );

   /*
      PMD Start type define
   */
   enum SDB_START_TYPE
   {
      SDB_START_NORMAL  = 0,
      SDB_START_CRASH
   } ;

   /*
      Register db to krcb
   */
   #define PMD_REGISTER_CB( pCB ) pmdGetKRCB()->registerCB( pCB )

   class _clsMgr ;
   class _clsReplicateSet ;
   class _clsShardMgr ;
   class _dpsLogWrapper ;
   class dpsTransCB ;
   class _SDB_DMSCB ;
   class _SDB_RTNCB ;
   class _bpsCB ;
   class sdbCatalogueCB ;
   class _CoordCB ;
   class _sqlCB ;
   class _authCB ;
   class aggrBuilder ;
   class _spdFMPMgr ;

   /*
    * Kernel Control Block
    * Database Kernel Variables
    */
   class _SDB_KRCB : public SDBObject
   {
   public:
      _SDB_KRCB () ;
      ~_SDB_KRCB () ;

      INT32 init () ;
      void  destroy () ;

      IControlBlock*    getCBByType( SDB_CB_TYPE type ) ;
      BOOLEAN           isCBValue( SDB_CB_TYPE type ) const ;

      INT32             registerCB( IControlBlock *pCB ) ;

   private:
      IControlBlock                 *_arrayCBs[ SDB_CB_MAX ] ;
      BOOLEAN                       _init ;

   private :
      // configured options
      CHAR           _groupName    [ OSS_MAX_GROUPNAME_SIZE + 1 ] ;
      SDB_ROLE       _role ;
      SDB_START_TYPE _startType ;

      UINT32         _dbStatus ;

      BOOLEAN        _businessOK ;
      INT32          _exitCode ;

      _pmdEDUMgr     _eduMgr ;

      _clsMgr        *_clsCB ;
      dpsTransCB     *_dpsTransCB ;
      _dpsLogWrapper *_dpscb ;
      _SDB_DMSCB     *_dmscb ;
      _SDB_RTNCB     *_rtncb ;
      _bpsCB         *_bpscb ;
      sdbCatalogueCB *_catlogueCB;
      _CoordCB       *_coordcb;
      _sqlCB         *_sql ;
      _authCB        *_auth ;
      _pdTraceCB     *_traceCB ;
      aggrBuilder    *_aggrCB;
      _spdFMPMgr     *_fmpCB ;

      monConfigCB    _monCfgCB ;
      monDBCB        _monDBCB ;
      _pmdOptionsMgr _optioncb ;
      ossTick        _curTime ;

   public :

      SDB_START_TYPE getStartType () const
      {
         return _startType ;
      }
      void setStartType ( SDB_START_TYPE startType )
      {
         _startType = startType ;
      }
      UINT32 getDBStatus () const
      {
         return _dbStatus ;
      }
      SDB_ROLE getDBRole () const
      {
         return _role ;
      }
      pmdEDUMgr *getEDUMgr ()
      {
         return &_eduMgr ;
      }
      const CHAR *getGroupName () const
      {
         return _groupName ;
      }
      CHAR *getGroupName ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL;
         ossStrncpy ( pBuffer, _groupName, size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      OSS_INLINE _SDB_DMSCB *getDMSCB ()
      {
         return _dmscb ;
      }
      OSS_INLINE _SDB_RTNCB *getRTNCB ()
      {
         return _rtncb ;
      }

      OSS_INLINE monConfigCB * getMonCB()
      {
         return & _monCfgCB ;
      }

      OSS_INLINE monDBCB * getMonDBCB ()
      {
         return &_monDBCB ;
      }
      OSS_INLINE _clsMgr *getClsCB ()
      {
         return _clsCB ;
      }
      _clsReplicateSet *getReplCB () ;
      _clsShardMgr *getShardCB () ;

      OSS_INLINE _dpsLogWrapper* getDPSCB ()
      {
         return _dpscb ;
      }
      OSS_INLINE _bpsCB *getBPSCB ()
      {
         return _bpscb ;
      }
      OSS_INLINE _pdTraceCB *getTraceCB ()
      {
         return _traceCB ;
      }
      OSS_INLINE _pmdOptionsMgr *getOptionCB()
      {
         return &_optioncb;
      }
      OSS_INLINE sdbCatalogueCB *getCATLOGUECB()
      {
         return _catlogueCB;
      }

      OSS_INLINE _CoordCB *getCoordCB()
      {
         return _coordcb ;
      }

      OSS_INLINE _sqlCB *getSqlCB()
      {
         return _sql ;
      }

      OSS_INLINE _authCB *getAuthCB()
      {
         return _auth ;
      }

      OSS_INLINE dpsTransCB *getTransCB()
      {
         return _dpsTransCB;
      }

      OSS_INLINE aggrBuilder *getAggrCB()
      {
         return _aggrCB;
      }

      OSS_INLINE _spdFMPMgr *getFMPCB()
      {
         return _fmpCB ;
      }

      INT32 isBusinessOK() const
      {
         return _businessOK ;
      }
      INT32 getExitCode() const
      {
         return _exitCode ;
      }
      void setExitCode( INT32 exitCode )
      {
         if ( exitCode && _exitCode )
         {
            return ;
         }
         _exitCode = exitCode ;
      }
      void setBusinessOK( BOOLEAN businessOK )
      {
         _businessOK = businessOK ;
      }
      void setDBStatus ( UINT32 status )
      {
         _dbStatus = status ;
      }
      void enforceDBRole ( SDB_ROLE role )
      {
         _role = role ;
         pmdSetDBRole( _role ) ;
      }
      void setMonCB( monConfigCB & monCB )
      {
         _monCfgCB = monCB ;
      }

      void setMonDBCB ( monDBCB & cb )
      {
         _monDBCB = cb ;
      }

      void setMonTimestampSwitch( BOOLEAN flag )
      {
          _monCfgCB.timestampON = flag ;
      }

      void enforceNodeInfo ( const _MsgRouteID &id , const CHAR * host ) ;
      void enforceReplAddr( UINT16 serviceID, const CHAR *replService ) ;
      void enforceShardAddr ( UINT16 serviceID, const CHAR *shardService ) ;
      void setGroupName ( const CHAR *groupName )
      {
         ossMemset( _groupName, 0, sizeof(_groupName) ) ;
         ossStrncpy ( _groupName, groupName, sizeof(_groupName)-1 );
      }
      void enforceCatAddr( const _MsgRouteID &id,
                           const CHAR *host,
                           const CHAR *service ) ;
      void updateCatRouteID ( const _MsgRouteID &id ) ;

      void enforceCataLogGrpAddrs( const _MsgRouteID &id,
                                   const CHAR *host,
                                   const CHAR *service ) ;

      void enforceLogFileSz ( UINT32 logFileSz ) ;
      void enforceLogFileNum ( UINT32 logFileNum ) ;

      ossTick getCurTime() ;
      void syncCurTime() ;

   } ;
   typedef class _SDB_KRCB pmdKRCB ;

   /*
    * Get global kernel control block
    * This variable is unique per process
    */
   extern pmdKRCB pmd_krcb ;
   OSS_INLINE pmdKRCB* pmdGetKRCB()
   {
      return &pmd_krcb ;
   }
   OSS_INLINE pmdOptionsCB* pmdGetOptionCB()
   {
      return pmdGetKRCB()->getOptionCB() ;
   }

}

#endif //PMD_HPP__

