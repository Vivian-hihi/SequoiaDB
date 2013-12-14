/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = monCB.hpp

   Descriptive Name = Monitor Control Block Header

   When/how to use: this program may be used on binary and text-formatted
   versions of monitoring component. This file contains structure for
   application and context snapshot.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MONCB_HPP_
#define MONCB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"

namespace engine
{
   struct _monConfigCB : public SDBObject
   {
      BOOLEAN timestampON ;
   } ;
   typedef struct _monConfigCB monConfigCB ;


   enum MON_OPERATION_TYPES
   {
      MON_COUNTER_OPERATION_NONE = 0,
      MON_DATA_READ,
      MON_INDEX_READ,
      MON_TEMP_READ,
      MON_DATA_WRITE,
      MON_INDEX_WRITE,
      MON_TEMP_WRITE,
      MON_UPDATE,
      MON_DELETE,
      MON_INSERT,
      MON_UPDATE_REPL,
      MON_DELETE_REPL,
      MON_INSERT_REPL,
      MON_SELECT,
      MON_READ,
      MON_COUNTER_OPERATION_MAX = MON_READ,

      MON_TIME_OPERATION_NONE = 0,
      MON_TOTAL_READ_TIME,
      MON_TOTAL_WRITE_TIME,
      MON_TIME_OPERATION_MAX = MON_TOTAL_WRITE_TIME
   } ;

   class _monDBCB : public SDBObject
   {
   public :
      UINT32 numConnects ;

      UINT64 totalDataRead ;
      UINT64 totalIndexRead ;
      UINT64 totalDataWrite ;
      UINT64 totalIndexWrite ;

      UINT64 totalUpdate ;
      UINT64 totalDelete ;
      UINT64 totalInsert ;
      UINT64 totalSelect ;  // total records into result set
      UINT64 totalRead ;    // total records readed from disk
      UINT32 receiveNum ;

      UINT64 replUpdate ;   // IUD caused by replica copy
      UINT64 replDelete ;
      UINT64 replInsert ;

      ossTickDelta totalReadTime ;
      ossTickDelta totalWriteTime ;
      ossTick      _activateTimeStampTick ;
      ossTimestamp _activateTimestamp ;
      void monOperationTimeInc( MON_OPERATION_TYPES op, ossTickDelta &delta )
      {
         switch ( op )
         {
            case MON_TOTAL_READ_TIME :
               totalReadTime += delta ;
               break ;

            case MON_TOTAL_WRITE_TIME :
               totalWriteTime += delta ;
               break ;

            default :
               break ;
         }
      }

      void monOperationCountInc( MON_OPERATION_TYPES op, UINT64 delta = 1 )
      {
         switch ( op )
         {
            case MON_DATA_READ :
               totalDataRead += delta ;
               break ;

            case MON_INDEX_READ :
               totalIndexRead += delta ;
               break ;

            case MON_DATA_WRITE :
               totalDataWrite += delta ;
               break ;

            case MON_INDEX_WRITE :
               totalIndexWrite += delta ;
               break ;

            case MON_UPDATE :
               totalUpdate += delta ;
               break ;

            case MON_DELETE :
               totalDelete += delta ;
               break ;

            case MON_INSERT :
               totalInsert += delta ;
               break ;

            case MON_UPDATE_REPL :
               replUpdate += delta ;
               break ;

            case MON_DELETE_REPL :
               replDelete += delta ;
               break ;

            case MON_INSERT_REPL :
               replInsert += delta ;
               break ;

            case MON_SELECT :
               totalSelect += delta ;
               break ;

            case MON_READ :
               totalRead += delta ;
               break ;

            default:
               break ;
         }
      }

      void reset()
      {
         numConnects     = 0 ;

         totalDataRead   = 0 ;
         totalIndexRead  = 0 ;
         totalDataWrite  = 0 ;
         totalIndexWrite = 0 ;

         totalUpdate     = 0 ;
         totalDelete     = 0 ;
         totalInsert     = 0 ;
         totalSelect     = 0 ;
         totalRead       = 0 ;

         replUpdate      = 0 ;
         replInsert      = 0 ;
         replDelete      = 0 ;

         totalReadTime.clear() ;
         totalWriteTime.clear() ;
      }

      UINT32 getReceiveNum ()
      {
         return receiveNum ;
      }
      void addReceiveNum ()
      {
         ++receiveNum ;
      }

      _monDBCB()
      {
         reset() ;
         receiveNum = 0 ;
         _activateTimeStampTick.clear() ;
         _activateTimestamp.time = 0 ;
         _activateTimestamp.microtm = 0 ;
      }

      _monDBCB &operator= ( const _monDBCB &rhs )
      {
         numConnects               = rhs.numConnects ;

         totalDataRead             = rhs.totalDataRead ;
         totalIndexRead            = rhs.totalIndexRead ;
         totalDataWrite            = rhs.totalDataWrite ;
         totalIndexWrite           = rhs.totalIndexWrite ;

         totalUpdate               = rhs.totalUpdate ;
         totalDelete               = rhs.totalDelete ;
         totalInsert               = rhs.totalInsert ;
         totalSelect               = rhs.totalSelect ;
         totalRead                 = rhs.totalRead ;

         replUpdate                = rhs.replUpdate ;
         replDelete                = rhs.replDelete ;
         replInsert                = rhs.replInsert ;

         totalReadTime             = rhs.totalReadTime ;
         totalWriteTime            = rhs.totalWriteTime ;
         _activateTimestamp.time    = rhs._activateTimestamp.time;
         _activateTimestamp.microtm = rhs._activateTimestamp.microtm ;
         _activateTimeStampTick     = rhs._activateTimeStampTick ;

         return *this ;
      }

      void recordActivateTimestamp()
      {
         _activateTimeStampTick.sample() ;
         ossGetCurrentTime( _activateTimestamp ) ;
      }

   } ;
   typedef class _monDBCB  monDBCB ;

   class _monAppCB : public SDBObject
   {
   public :
      monDBCB *mondbcb ;
      UINT64 totalDataRead ;
      UINT64 totalIndexRead ;
      UINT64 totalDataWrite ;
      UINT64 totalIndexWrite ;

      UINT64 totalUpdate ;
      UINT64 totalDelete ;
      UINT64 totalInsert ;
      UINT64 totalSelect ;  // total records into result set
      UINT64 totalRead ;    // total records readed from disk

      ossTickDelta totalReadTime ;
      ossTickDelta totalWriteTime ;
      ossTick      _connectTimeStampTick ;
      ossTimestamp _connectTimestamp ;

      void monOperationTimeInc( MON_OPERATION_TYPES op, ossTickDelta &delta )
      {
         switch ( op )
         {
            case MON_TOTAL_READ_TIME :
               totalReadTime += delta ;
               break ;

            case MON_TOTAL_WRITE_TIME :
               totalWriteTime += delta ;
               break ;

            default :
               break ;
         }
         mondbcb->monOperationTimeInc ( op, delta ) ;
      }

      void monOperationCountInc( MON_OPERATION_TYPES op, UINT64 delta = 1 )
      {
         switch ( op )
         {
            case MON_DATA_READ :
               totalDataRead += delta ;
               break ;

            case MON_INDEX_READ :
               totalIndexRead += delta ;
               break ;

            case MON_DATA_WRITE :
               totalDataWrite += delta ;
               break ;

            case MON_INDEX_WRITE :
               totalIndexWrite += delta ;
               break ;

            case MON_UPDATE :
               totalUpdate += delta ;
               break ;

            case MON_DELETE :
               totalDelete += delta ;
               break ;

            case MON_INSERT :
               totalInsert += delta ;
               break ;

            case MON_SELECT :
               totalSelect += delta ;
               break ;

            case MON_READ :
               totalRead += delta ;
               break ;

            default:
               break ;
         }
         mondbcb->monOperationCountInc ( op, delta ) ;
      }

      void reset()
      {
         totalDataRead = 0 ;
         totalIndexRead = 0 ;
         totalDataWrite = 0 ;
         totalIndexWrite = 0 ;

         totalUpdate = 0 ;
         totalDelete = 0 ;
         totalInsert = 0 ;
         totalSelect = 0 ;
         totalRead  = 0 ;

         totalReadTime.clear() ;
         totalWriteTime.clear() ;
         _connectTimeStampTick.clear() ;
         _connectTimestamp.time = 0 ;
         _connectTimestamp.microtm = 0 ;
      }

      _monAppCB() ;

      _monAppCB &operator= ( const _monAppCB &rhs ) ;

      void recordConnectTimestamp()
      {
         _connectTimeStampTick.sample() ;
         ossGetCurrentTime( _connectTimestamp ) ;
      }

   } ;
   typedef class _monAppCB  monAppCB ;

   class _monContextCB : public SDBObject
   {
   public :
      UINT64 dataRead ;
      UINT64 indexRead ;
      ossTickDelta queryTimeSpent ;
      ossTimestamp _startTimestamp ;
      ossTick      _startTimestampTick ;

      void reset()
      {
         dataRead = 0 ;
         indexRead = 0 ;
         queryTimeSpent.clear() ;
         _startTimestamp.time = 0 ;
         _startTimestamp.microtm = 0 ;
         _startTimestampTick.clear() ;
      }

      _monContextCB()
      {
         reset() ;
      }

      _monContextCB &operator= ( const _monContextCB &rhs )
      {
         dataRead                = rhs.dataRead ;
         indexRead               = rhs.indexRead ;
         queryTimeSpent          = rhs.queryTimeSpent ;
         _startTimestamp.time    = rhs._startTimestamp.time ;
         _startTimestamp.microtm = rhs._startTimestamp.microtm ;
         _startTimestampTick     = rhs._startTimestampTick ;
         return *this ;
      }

      void recordStartTimestamp()
      {
         _startTimestampTick.sample() ;
         ossGetCurrentTime( _startTimestamp ) ;
      }

      void monOperationCountInc( MON_OPERATION_TYPES op, UINT64 delta = 1 )
      {
         switch ( op )
         {
            case MON_DATA_READ :
               dataRead += delta ;
               break ;

            case MON_INDEX_READ :
               indexRead += delta ;
               break ;

            default:
               break ;
         }
      }

      void monOperationTimeInc( MON_OPERATION_TYPES op, ossTickDelta &delta )
      {
         queryTimeSpent += delta ;
      }

   } ;
   typedef class _monContextCB  monContextCB ;
}
#endif
