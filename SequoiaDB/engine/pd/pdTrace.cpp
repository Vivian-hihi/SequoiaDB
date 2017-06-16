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

   Source File Name = pdTrace.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/1/2014  ly  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "pdTraceAnalysis.hpp"
#include "pd.hpp"
#if defined (SDB_ENGINE)
#include "pmd.hpp"
#include "pmdDef.hpp"
#include "pmdEDU.hpp"
#include "pmdEDUMgr.hpp"
#endif
#include "ossLatch.hpp"
#include "ossPrimitiveFileOp.hpp"
#include <math.h>
#include <stack>

using namespace engine ;

ossSpinXLatch gPDTraceMutex;
ossSpinXLatch gPDTraceReserveMutex;
// extract high 32 bit as function component mask, and OR with
// cb->_componentMask, if the result is 0 that means the component is not what
// we want

static BOOLEAN isThreadMonited( UINT32 tid, pdTraceCB *cb )
{
   BOOLEAN isMonited = FALSE;
   INT32 idx = 0 ;

   do 
   {
      if ( cb->_threadmonitorStart.compare(FALSE) )
      {
         isMonited = TRUE ;
         break;
      }
      for (; idx < cb->_nMonitoredNum; idx++)
      {
         if (cb->_monitoredThreads[idx] == tid) 
         {
            isMonited = TRUE;
            break;
         }
      }
   } while (0);

   return isMonited ;
}
static BOOLEAN pdTraceMask ( UINT64 funcCode, pdTraceCB *cb )
{
   SDB_ASSERT ( cb, "trace cb can't be NULL" ) ;
   UINT32 component = (UINT32)(funcCode>>32) ;
   return ( component&cb->_componentMask ) != 0 ;
}
#if defined (SDB_ENGINE)
#define PD_TRACE_PAUSE_DFT_WAIT 100
void _pdTraceCB::pause ( UINT64 funcCode )
{
   pmdEDUCB *educb    = NULL ;
   pmdEDUEvent event;

   // compare each defined break points
   for ( UINT32 i = 0; i < _numBP; ++i )
   {
      // if the break point matches our current function code
      if ( _bpList[i] == funcCode )
      {
         educb    = pmdGetKRCB()->getEDUMgr()->getEDU();
         // put EDU into pause status
         addPausedEDU ( educb ) ;

         educb->waitEvent( engine::PMD_EDU_EVENT_BP_RESUME, event, -1 ) ;
         break ;
      } // if ( _bpList[i] == funcCode )
   } // for ( i = 0; i < _numBP; ++i )
}
#endif

void pdTraceFunc ( UINT64 funcCode, INT32 type,
                   const CHAR* file, UINT32 line,
                   pdTraceArgTuple *tuple )
{
   // make sure trace is turned on
   if ( sdbGetPDTraceCB()->_traceStarted.compare(FALSE) )
      return ;
   // make sure the function is what we want
   if ( !pdTraceMask ( funcCode, sdbGetPDTraceCB() ) )
      return ;
   UINT32 tid;
   pdTraceRecord record ;
   void *pBuffer = NULL ;
   UINT32 code = (UINT32)funcCode&0xFFFFFFFF;


   tid = ossGetCurrentThreadID () ;
   if ( !isThreadMonited( tid, sdbGetPDTraceCB() ) )
      return;

   ossMemcpy ( record._eyeCatcher, TRACE_EYE_CATCHER,
               TRACE_EYE_CATCHER_SIZE ) ;
   record._recordSize = sizeof(record) ;
   record._functionID = code ;
   record._flag       = (UINT8)type ;
   record._tid        = tid ;
   record._line       = (UINT16)line ;
   record._numArgs    = 0 ;
   ossGetCurrentTime ( record._timestamp ) ;

   // parse arguments and calcualte the total size of buffer we need
   for ( INT8 i = 0; i < PD_TRACE_MAX_ARG_NUM; ++i )
   {
      if ( PD_TRACE_ARGTYPE_NONE != tuple[i].x )
      {
         ++record._numArgs ;
         record._recordSize += tuple[i].z +
                               sizeof(tuple[i].x) +
                               sizeof(tuple[i].z);
      }
   }

   pBuffer = sdbGetPDTraceCB()->reserveMemory( record._recordSize ) ;
   if ( !pBuffer )
      goto done ;
   sdbGetPDTraceCB()->startWrite () ;
   // fill trace buffer
   pBuffer = sdbGetPDTraceCB()->fillIn ( pBuffer, &record, sizeof(record) ) ;
   for ( INT8 i = 0; i < PD_TRACE_MAX_ARG_NUM; ++i )
   {
      if ( PD_TRACE_ARGTYPE_NONE != tuple[i].x )
      {
         pBuffer = sdbGetPDTraceCB()->fillIn ( pBuffer, &tuple[i].x,
                                               sizeof(tuple[i].x) ) ;
         pBuffer = sdbGetPDTraceCB()->fillIn ( pBuffer, &tuple[i].z,
                                               sizeof(tuple[i].z) ) ;
         // size includes pdTraceArgument head, so we have to copy the rest of
         // data
         pBuffer = sdbGetPDTraceCB()->fillIn ( pBuffer, tuple[i].y,
                                               tuple[i].z-sizeof(pdTraceArgument) ) ;
      }
   }
   sdbGetPDTraceCB()->finishWrite () ;
#if defined (SDB_ENGINE)
   if ( sdbGetPDTraceCB()->_numBP )
      sdbGetPDTraceCB()->pause ( code ) ;
#endif
done :
   return ;
}

_pdTraceCB::_pdTraceCB()
:_traceStarted(FALSE),
 _currentWriter(0),
 _componentMask(0xFFFFFFFF),
 _totalSize(0),
 _freeBlockHead(0),
 _freeBlockTail(0),
 _threadmonitorStart(FALSE),
 _nMonitoredNum(0),
 _pBuffer(NULL)
{
   _headerSize = sizeof(_pdTraceCB) ;
   ossMemcpy ( _eyeCatcher, TRACECB_EYE_CATCHER,
               TRACE_EYE_CATCHER_SIZE ) ;
}

// free memory
_pdTraceCB::~_pdTraceCB()
{
   reset () ;
}


void * _pdTraceCB::reserveMemory(UINT32 size)
{
   ossValuePtr reservePtr = NULL;
   CHAR buff[TRACE_SLOT_SIZE] ;
   _pdTraceRecord *pRecord = NULL;
   UINT64 freeSize;

   gPDTraceReserveMutex.get() ;

   reservePtr = (ossValuePtr)_pBuffer + _freeBlockHead ;
   if ( _freeBlockHead < _freeBlockTail )
   {
      freeSize = _freeBlockTail - _freeBlockHead ;
   }
   else
   {
      freeSize = (_freeBlockTail + _totalSize - _freeBlockHead) % _totalSize ;
   }

   if ( freeSize < size )
   {
      do 
      {
         if ( _freeBlockTail + sizeof(_pdTraceRecord) > _totalSize )
         {
            ossMemcpy ( buff, (CHAR*)((ossValuePtr)_pBuffer + _freeBlockTail), _totalSize-_freeBlockTail ) ;
            ossMemcpy ( buff + _totalSize-_freeBlockTail, _pBuffer, sizeof(_pdTraceRecord)+_freeBlockTail-_totalSize) ;
            pRecord = (_pdTraceRecord *)&buff ;
         }
         else
         {
            pRecord = (_pdTraceRecord *)((ossValuePtr)_pBuffer + _freeBlockTail) ;
         }
         _freeBlockTail = (_freeBlockTail + pRecord->_recordSize) % _totalSize ;
         freeSize += pRecord->_recordSize ;
      } while ( freeSize < size );
   }

   _freeBlockHead = (_freeBlockHead + size) % _totalSize ;

   gPDTraceReserveMutex.release() ;
   return (void*)reservePtr ;
}

void* _pdTraceCB::fillIn ( void *pPos, const void *pInput, INT32 size )
{
   CHAR *pRetAddr        = NULL ;
   ossValuePtr posStart  = 0 ;
   ossValuePtr posEnd    = 0 ;
   SDB_ASSERT ( pPos && pInput, "pos and input can't be NULL" ) ;
   // target offset must be in valid range
   SDB_ASSERT ( pPos >= _pBuffer, "pos can't be smaller than buffer" ) ;
   // if we are asked to write too big data, let's just skip it
   if ( size < 0 || size >= TRACE_RECORD_MAX_SIZE )
   {
      pRetAddr = (CHAR*)pPos ;
      goto done ;
   }
   posStart = (ossValuePtr)_pBuffer +
                 (((ossValuePtr)pPos-(ossValuePtr)_pBuffer) %
                 _totalSize ) ;
   posEnd = (ossValuePtr)_pBuffer +
                 (((ossValuePtr)pPos-(ossValuePtr)_pBuffer + size) %
                 _totalSize ) ;
   if ( posStart > posEnd )
   {
      // if the message need to be wrapped, let's break it to two parts ( we
      // already ensured the input won't be larger than buffer )
      // diff is posEnd - _pBuffer, which is how many bytes we need to write to
      // the wrapped part, that means size-diff will be the part at end of
      // buffer
      INT32 diff = posEnd - (ossValuePtr)_pBuffer ;
      // copy first part into end
      ossMemcpy ( pPos, pInput, size-diff ) ;
      // copy rest part to begin
      ossMemcpy ( _pBuffer, (CHAR*)pInput+size-diff, diff ) ;
      pRetAddr = _pBuffer + diff ;
   }
   else
   {
      // if we won't wrap, let's just write whatever to buffer
      ossMemcpy ( pPos, pInput, size ) ;
      pRetAddr = (CHAR*)((ossValuePtr)pPos + size) ;
   }
done :
   return pRetAddr ;
}

void _pdTraceCB::startWrite ()
{
   _currentWriter.inc() ;
}

void _pdTraceCB::finishWrite ()
{
   _currentWriter.dec() ;
}

void _pdTraceCB::setMask ( UINT32 mask )
{
   _componentMask = mask ;
}

UINT32 _pdTraceCB::getMask ()
{
   return _componentMask ;
}

INT32 _pdTraceCB::start ( UINT64 size )
{
   return start ( size, 0xFFFFFFFF, NULL ) ;
}

INT32 _pdTraceCB::start ( UINT64 size, UINT32 mask )
{
   return start ( size, mask, NULL ) ;
}

INT32 _pdTraceCB::start ( UINT64 size, UINT32 mask,
                          std::vector<UINT64> *funcCode )
{
   INT32 rc = SDB_OK ;
   gPDTraceMutex.get() ;
   // sanity check, make sure we are not currently tracing anything
   if ( _traceStarted.compare ( TRUE ) )
   {
      PD_LOG ( PDWARNING, "Trace is already started" ) ;
      rc = SDB_PD_TRACE_IS_STARTED ;
      goto error ;
   }
   reset () ;
#if defined (SDB_ENGINE)
   // trace stop break
   if ( funcCode )
   {
      for ( UINT32 i = 0; i < funcCode->size(); ++i )
      {
         rc = addBreakPoint ( (*funcCode)[i] ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to add break point, rc = %d", rc ) ;
            removeAllBreakPoint() ;
            goto error ;
         }
      }
   }
#endif
#ifdef THREADMONITOR
   std::vector<UINT32> tids;
   _nMonitoredNum = tids.size() ;
   if ( _nMonitoredNum > PD_TRACE_MAX_MONITORED_THREAD_NUM )
   {
      PD_LOG ( PDWARNING, "too many threads monitored (up to 10)" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   else if ( _nMonitoredNum )
   {

      _threadmonitorStart.init( TRUE ) ;
      for ( UINT32 i = 0; i < _nMonitoredNum; i++ )
      {
         _monitoredThreads[i] = tids[i] ;
      }
   }
#endif
   // start trace
   PD_LOG ( PDEVENT, "Trace starts, buffersize = %llu, mask = 0x%x",
            size, mask ) ;
   size           = ossRoundUpToMultipleX ( size, TRACE_CHUNK_SIZE ) ;
   size           = OSS_MAX ( size, TRACE_MIN_BUFFER_SIZE ) ;
   size           = OSS_MIN ( size, TRACE_MAX_BUFFER_SIZE ) ;

   _totalSize = size ;
   _freeBlockHead = 0;
   _freeBlockTail = _totalSize;
   // memory will be freed in destructor or reset
   _pBuffer       = (CHAR*)SDB_OSS_MALLOC ( (size_t)size ) ;
   if ( _pBuffer )
   {
      _componentMask = mask ;
      _traceStarted.init ( TRUE ) ;
   }
   else
   {
      PD_LOG ( PDERROR,
               "Failed to allocate memory for trace buffer: %lld bytes",
               size ) ;
   }
done :
   gPDTraceMutex.release() ;
   return rc ;
error :
   goto done ;
}

void _pdTraceCB::stop ()
{
   PD_LOG ( PDEVENT, "Trace stops" ) ;
   _traceStarted.compareAndSwap ( TRUE, FALSE ) ;
}

INT32 _pdTraceCB::dump ( const CHAR *pFileName )
{
   INT32 rc = SDB_OK ;
   ossPrimitiveFileOp file ;
   UINT32 totalChunks;

   gPDTraceMutex.get () ;
   PD_CHECK ( _traceStarted.compare ( FALSE ), SDB_PD_TRACE_IS_STARTED,
              error, PDWARNING,
              "Trace must be stopped before dumping" ) ;
   PD_CHECK ( _pBuffer, SDB_PD_TRACE_HAS_NO_BUFFER, error, PDWARNING,
              "Trace buffer does not exist, trace must be captured "
              "before dump" ) ;
   // wait until all writers finish writing
   while ( !_currentWriter.compare(0) ) ;
   PD_LOG ( PDEVENT, "Trace dumps to %s", pFileName ) ;
   // attempt to open the file
   rc = file.Open ( pFileName,
                    OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                    OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                    OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              pFileName, rc ) ;

   // write traceCB first
   rc = file.Write ( this, sizeof(_pdTraceCB) ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to write trace to file %s, errno=%d",
              pFileName, rc ) ;
   // write buffer
   totalChunks = _totalSize/TRACE_CHUNK_SIZE ;
   for ( UINT32 i = 0; i < totalChunks; ++i )
   {
      rc = file.Write ( &_pBuffer[i*TRACE_CHUNK_SIZE],
                        TRACE_CHUNK_SIZE ) ;
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                 "Failed to write trace to file %s, errno=%d",
                 pFileName, rc ) ;
   }
   if ( _pBuffer )
   {
      SDB_OSS_FREE ( _pBuffer ) ;
      _pBuffer = NULL ;
   }
   
done :
   file.Close () ;
   gPDTraceMutex.release () ;
   return rc ;
error :
   goto done ;
}



INT32 loadDumpFile( const CHAR *pInputFileName ,
                    ossPrimitiveFileOp *file ,
                    _pdTraceCB &traceCB 
                    )
{
   INT32 rc               = SDB_OK ;
   INT32 byteRead         = 0 ;
   UINT64 fileSize        = 0 ;
   ossPrimitiveFileOp::offsetType offset ;

   rc = file->Open ( pInputFileName,
      OSS_PRIMITIVE_FILE_OP_READ_ONLY |
      OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
      "Failed to dump trace to file %s, errno=%d",
      pInputFileName, rc ) ;
   rc = file->getSize ( &offset ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
      "Failed to get trace file size for %s, errno=%d",
      pInputFileName, rc ) ;
   fileSize = offset.offset ;
   // open output file for write only

   // read traceCB first
   rc = file->Read ( sizeof(_pdTraceCB), &traceCB, &byteRead ) ;
   // reset buffer since it's fake
   traceCB._pBuffer = NULL ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
      "Failed to write trace to file %s, errno=%d",
      pInputFileName, rc ) ;
   PD_CHECK ( byteRead == sizeof(_pdTraceCB),
      SDB_PD_TRACE_FILE_INVALID, error, PDWARNING,
      "Unable to read header" ) ;
   PD_CHECK ( ossMemcmp ( traceCB._eyeCatcher, TRACECB_EYE_CATCHER,
      TRACE_EYE_CATCHER_SIZE ) == 0,
      SDB_PD_TRACE_FILE_INVALID, error, PDWARNING,
      "Invalid eye catcher" ) ;
   // if the header size doesn't match _pdTraceCB, it may caused by
   // server/client differnece for pdTraceCB, so let's read rest
   if ( traceCB._totalSize + traceCB._headerSize != fileSize )
   {
      PD_LOG ( PDWARNING,
         "Trace file header is not valid, should be greater "
         "or equal to %lld bytes, but actual %lld bytes",
         sizeof(_pdTraceCB), traceCB._headerSize ) ;
      goto error ;
   }
   if ( sizeof(_pdTraceCB) < traceCB._headerSize )
   {
      ossPrimitiveFileOp::offsetType off ;
      off.offset = traceCB._headerSize ;
      file->seekToOffset ( off ) ;
   }
   else if ( sizeof(_pdTraceCB) > traceCB._headerSize )
   {
      PD_LOG ( PDWARNING,
         "Trace file header is not valid, should be greater "
         "or equal to %lld bytes, but actual %lld bytes",
         sizeof(_pdTraceCB), traceCB._headerSize ) ;
      goto error ;
   }

done :
   return rc ;
error :
   goto done ;
}

void removeSuffix(char *path)
{
   int idx = ossStrlen(path)-1;
   while(idx >= 0 && path[idx] != '\\' && path[idx] != '/')
   {
      if (path[idx] == '.')
      {
         path[idx] = 0;
         break;
      }
      idx--;
   }
}

// load external dumped file and format
INT32 _pdTraceCB::format ( const CHAR *pInputFileName,
                           const CHAR *pOutputFileName,
                           _pdTraceFormatType type )
{
   INT32 rc               = SDB_OK ;

   _pdTraceCB traceCB ;
   ossPrimitiveFileOp file ;
   CHAR fmtFilePath[PATH_MAX] = {0} ;
   CHAR flwFilePath[PATH_MAX] = {0} ;
   CHAR summaryFilePath[PATH_MAX] = {0} ; 
   CHAR exceptFilePath[PATH_MAX] = {0} ;
   CHAR errorFilePath[PATH_MAX] = {0} ;
   CHAR funcRecordFilePath[PATH_MAX] = {0} ;
   CHAR filePath[PATH_MAX] = {0} ;
   std::map<UINT32, std::vector<TraceRecordIndex> > tid2recordsMap ;

   /*ossStrcpy( filePath, pOutputFileName ) ; 
   removeSuffix( filePath ) ;

   if ( type == PD_TRACE_FORMAT_TYPE_FORMAT )
   {
      ossSnprintf( fmtFilePath, PATH_MAX, "%s.fmt", filePath ) ;
   }
   else
   {
      ossSnprintf( flwFilePath, PATH_MAX, "%s.flw", filePath ) ;
      ossSnprintf( exceptFilePath, PATH_MAX, "%s.except", filePath ) ;
      ossSnprintf( summaryFilePath, PATH_MAX, "%s.sumary", filePath ) ;
      ossSnprintf( errorFilePath, PATH_MAX, "%s.error", filePath ) ;
      ossSnprintf( funcRecordFilePath, PATH_MAX, "%s.funcRecord.csv", filePath ) ;

   }*/


   if ( type == PD_TRACE_FORMAT_TYPE_FORMAT )
   {
      ossSnprintf( fmtFilePath, PATH_MAX, "%s/trace.fmt", pOutputFileName ) ;
   }
   else
   {
      ossSnprintf( flwFilePath, PATH_MAX, "%s/trace.flw", pOutputFileName ) ;
      ossSnprintf( exceptFilePath, PATH_MAX, "%s/trace.except", pOutputFileName ) ;
      ossSnprintf( summaryFilePath, PATH_MAX, "%s/trace.sumary", pOutputFileName ) ;
      ossSnprintf( errorFilePath, PATH_MAX, "%s/trace.error", pOutputFileName ) ;
      ossSnprintf( funcRecordFilePath, PATH_MAX, "%s/trace.funcRecord.csv", pOutputFileName ) ;

   }

   if( ossAccess( pOutputFileName ) )
   {
      rc = ossMkdir( pOutputFileName ) ;
      PD_CHECK ( 0 == rc || SDB_PERM == rc, rc, error, PDERROR, "Failed to mkdir(%s), errno=%d", pOutputFileName, rc ) ;
   }

   rc = loadDumpFile( pInputFileName, &file, traceCB ) ;
   PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to load dump file(%s), errno=%d", pInputFileName, rc ) ;

   //step 1
   rc = parseTraceDumpFile( &file, &traceCB, fmtFilePath, tid2recordsMap );
   PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to parse file(%s), errno=%d", pInputFileName, rc ) ;

   if ( type != PD_TRACE_FORMAT_TYPE_FORMAT )
   {
      rc = analysisTraceRecords( &file, &traceCB, tid2recordsMap, 
                                 errorFilePath, funcRecordFilePath, flwFilePath, summaryFilePath, exceptFilePath );
      PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to analysis trace records, errno=%d", rc ) ;
   }


 
   
done :
   file.Close () ;
   return rc ;
error :
   goto done ;
}

void _pdTraceCB::destroy ()
{
   gPDTraceMutex.get() ;
   reset () ;
   gPDTraceMutex.release() ;
}

void _pdTraceCB::reset ()
{
   // stop trace if it's already started
   _traceStarted.compareAndSwap ( TRUE, FALSE ) ;
   // wait until there's no one write into the buffer
   while ( !_currentWriter.compare(0) ) ;
   if ( _pBuffer )
   {
      SDB_OSS_FREE ( _pBuffer ) ;
      _pBuffer = NULL ;
   }
   _traceStarted.init ( FALSE ) ;
   _freeBlockHead = 0 ;
   _freeBlockTail = 0 ;
   _totalSize = 0 ;
   _currentWriter.init ( 0 ) ;
   _componentMask = 0xFFFFFFFF ;
   ossMemcpy ( _eyeCatcher, TRACECB_EYE_CATCHER,
               TRACE_EYE_CATCHER_SIZE ) ;
#if defined (SDB_ENGINE)
   removeAllBreakPoint() ;
   if ( !_pmdEDUCBList.empty() )
   {
      ossSleepsecs ( 1 ) ;
      resumePausedEDUs () ;
   }
#endif
}

#if defined (SDB_ENGINE)
INT32 _pdTraceCB::addBreakPoint( UINT64 functionCode )
{
   INT32 rc = SDB_OK ;
   // duplicate detection
   for ( UINT32 i = 0; i < _numBP; ++i )
   {
      if ( functionCode == _bpList[i] )
         goto done ;
   }
   // here we need to insert the function code into list
   // first we have to make sure we still have enough room
   if ( _numBP >= PD_TRACE_MAX_BP_NUM )
   {
      rc = SDB_TOO_MANY_TRACE_BP ;
      goto error ;
   }
   _bpList[_numBP] = functionCode ;
   ++_numBP ;
done :
   return rc ;
error :
   goto done ;
}

void _pdTraceCB::removeAllBreakPoint()
{
   _numBP = 0 ;
   ossMemset ( &_bpList[0], 0, sizeof(_bpList) ) ;
}

void _pdTraceCB::addPausedEDU ( engine::_pmdEDUCB *cb )
{
   _pmdEDUCBLatch.get() ;
   _pmdEDUCBList.push_back ( cb ) ;
   _pmdEDUCBLatch.release () ;
}
void _pdTraceCB::resumePausedEDUs ()
{
   // if resume is called during db shutdown, let's simply ignore since all
   // other threads are going to detect the shutdown status and terminate
   // themselves
   if ( PMD_IS_DB_DOWN() )
      return ;
   _pmdEDUCBLatch.get() ;
   pmdEDUEvent event ( PMD_EDU_EVENT_BP_RESUME ) ;
   while ( !_pmdEDUCBList.empty() )
   {
      _pmdEDUCBList.front()->postEvent ( event ) ;
      _pmdEDUCBList.pop_front () ;
   }
   _pmdEDUCBLatch.release () ;
}

INT32 _pdTraceCB::getCurrent()
{
   return (INT32)_freeBlockHead;
}


#endif // SDB_ENGINE

/*
   get global pdtrace cb
*/
pdTraceCB* sdbGetPDTraceCB ()
{
   static pdTraceCB s_pdTraceCB ;
   return &s_pdTraceCB ;
}




