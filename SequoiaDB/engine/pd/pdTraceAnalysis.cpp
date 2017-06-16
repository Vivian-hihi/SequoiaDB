#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "pdTraceAnalysis.h"
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

INT32 selectExceptRecords(FunctionSummaryRecord &record, std::set<FunctionRecord> &exceptRecords) ;
INT32 outputFunctionSummaryRecord(UINT64 funcId, FunctionSummaryRecord &record, ossPrimitiveFileOp *funcFile) ;
INT32  outputExceptionReport(pdTraceCB *cb,
                             ossPrimitiveFileOp *dumpFile, 
                             ossPrimitiveFileOp *reportFile, 
                             std::set<FunctionRecord> &exceptRecords,
                             std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap) ;



INT32 outputTraceRecordByFMT(ossPrimitiveFileOp *out, CHAR *tempBuf, UINT32 sequenceNum);
INT32 outputTraceRecordByThread(ossPrimitiveFileOp *flwFile, 
                                UINT32 sequence,
                                INT32 &numIndent,
                                UINT64 timeInterval,
                                CHAR    *recordBuf);
INT32 readTraceRecord( ossPrimitiveFileOp *file,
                      UINT64 &cursor,
                      UINT32 headerSize,
                      UINT64 fileLength,
                      CHAR   *tempBuf); 

INT32 readFileData(ossPrimitiveFileOp *file, 
                   CHAR *pbuff, 
                   UINT64 &cursor, 
                   INT32  size, 
                   UINT32 headerSize, 
                   UINT64 fileLength); 

INT32 readTraceRecord(ossPrimitiveFileOp *file, 
                      UINT64 &cursor, 
                      UINT32 headerSize,
                      UINT64 fileLength,
                      CHAR   *tempBuf)
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( file, "file can't be NULL" ) ;
   pdTraceRecord *record = NULL ;

   rc = readFileData( file, &tempBuf[0], cursor, sizeof(pdTraceRecord), headerSize, fileLength ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to read from trace file, errno=%d", rc ) ;
   record = (pdTraceRecord*)tempBuf ;

   if ( record->_recordSize > sizeof(pdTraceRecord) )
   {
      rc = readFileData( file, &tempBuf[sizeof(pdTraceRecord)], cursor, record->_recordSize-sizeof(pdTraceRecord), headerSize, fileLength ) ;
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to read from trace file, errno=%d", rc ) ;
   }

   // sanity check for slot
   if ( ossMemcmp ( record->_eyeCatcher,
                    TRACE_EYE_CATCHER,
                    TRACE_EYE_CATCHER_SIZE ) != 0 )
   {
      // the slot does not start from eye catcher, that means it may not be a
      // valid slot
      //goto done ;
      rc = SDB_PD_TRACE_FILE_INVALID ;
      PD_CHECK ( 0 == rc, SDB_PD_TRACE_FILE_INVALID, error, PDERROR, "trace file invalid, errno=%d", rc ) ;
   }
   PD_CHECK ( record->_recordSize <= TRACE_RECORD_MAX_SIZE,
              SDB_PD_TRACE_FILE_INVALID, error, PDERROR, "read failed" ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 readFileData(ossPrimitiveFileOp *file, 
                    CHAR *pbuff, 
                    UINT64 &cursor, 
                    INT32  size, 
                    UINT32 headerSize, 
                    UINT64 fileLength)
{
   INT32 rc = SDB_OK ;
   INT32 readSize;
   ossPrimitiveFileOp::offsetType offset;

   do 
   {

      offset.offset = cursor;
      file->seekToOffset ( offset ) ;
      if ( (INT32)(fileLength - cursor) > size )
      {
         rc = file->Read ( size, pbuff, &readSize ) ;
         cursor += readSize;
      }
      else
      {
         rc = file->Read ( fileLength - cursor, pbuff, &readSize ) ;
         if ( (INT32)(fileLength - cursor) != readSize )
         {
            break;
         }

         offset.offset = headerSize;
         file->seekToOffset ( offset ) ;
         rc = file->Read ( size - readSize, pbuff + readSize, &readSize ) ;
         cursor = headerSize + readSize;
      }
   } while (0);

   return rc;
}

 

INT32 parseTraceDumpFile(ossPrimitiveFileOp *file, 
                         pdTraceCB *cb, 
                         CHAR *fmtFilePath,
                         std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap
                         )
{
   INT32 rc = SDB_OK;
   BOOLEAN isFmt = TRUE ;
   ossPrimitiveFileOp fmtfile;
   UINT64 cursor, endPos ;
   CHAR tempBuf [ TRACE_RECORD_MAX_SIZE ] ;
   UINT32 sequenceNum = 0 ;
   TraceRecordIndex recIdx ;
   pdTraceRecord *record ;

   if ( ossStrlen(fmtFilePath) )
   {
      rc = fmtfile.Open ( fmtFilePath,
                   OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                   OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                   OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              fmtFilePath, rc ) ;
   }
   else
   {
      isFmt = FALSE ;
   }
   

   //read record
   cursor = cb->_freeBlockTail % cb->_totalSize + cb->_headerSize ;
   endPos = cb->_freeBlockHead % cb->_totalSize + cb->_headerSize ;


   do 
   {
      recIdx = TraceRecordIndex( sequenceNum, cursor ) ;
      rc = readTraceRecord( file, cursor, cb->_headerSize, cb->_headerSize+cb->_totalSize, tempBuf );
      if(rc) break;

      if (isFmt)
      {
         rc = outputTraceRecordByFMT( &fmtfile, tempBuf, sequenceNum ) ;
         if(rc) break;
      }
      else
      {
         record = (pdTraceRecord *)tempBuf ;
         tid2recordsmap[record->_tid].push_back( recIdx ) ;
      }

      sequenceNum++;
   } while ( !rc && cursor != endPos );


done :
   if (isFmt) fmtfile.Close () ;
   return rc ;
error :
   goto done ;
}

void  outputErrorFunctions(std::map<UINT64, std::vector<UINT32> > &errFunctions, ossPrimitiveFileOp *file)
{
   INT32 nCount = 0 ;
   for ( std::map<UINT64, std::vector<UINT32> >::iterator it = errFunctions.begin(); it != errFunctions.end(); it++ )
   {
      file->fWrite( "%d: %s \n       (", nCount, pdGetTraceFunction( it->first ) ) ;
      for (int item = 0; item < it->second.size(); item++)
      {
         file->fWrite("%d ", it->second[item]) ;
      }
      file->fWrite( ")"OSS_NEWLINE ) ;
      nCount++ ;
   }
}


INT32 analysisTraceRecords(ossPrimitiveFileOp *file, pdTraceCB *cb,
                      std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap, 
                      CHAR *errorFilePath, 
                      CHAR *funcRecordPath, 
                      CHAR *flwFilePath, 
                      CHAR *summaryFilePath, 
                      CHAR *exceptFilePath)
{
   INT32 rc = SDB_OK ;
   ossPrimitiveFileOp errFile ;
   ossPrimitiveFileOp funcRecFile ;
   ossPrimitiveFileOp flwFile ;
   //ossPrimitiveFileOp smryFile ;
   ossPrimitiveFileOp exceptFile ;
   std::map<UINT64, std::vector<UINT32> > errFunctions;
   std::map<UINT64, FunctionSummaryRecord> summaryRecords ;
   //create file
   rc = errFile.Open ( errorFilePath,
                   OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                   OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                   OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              errorFilePath, rc ) ;
   rc = funcRecFile.Open ( funcRecordPath,
                   OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                   OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                   OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              funcRecordPath, rc ) ;

   rc = flwFile.Open ( flwFilePath,
                   OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                   OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                   OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              flwFilePath, rc ) ;

   rc = exceptFile.Open ( exceptFilePath,
                   OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                   OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                   OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              exceptFilePath, rc ) ;
   /*rc = smryFile.Open ( summaryFilePath,
                   OSS_PRIMITIVE_FILE_OP_WRITE_ONLY |
                   OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS |
                   OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to dump trace to file %s, errno=%d",
              summaryFilePath, rc ) ;*/



   //analysis thread
   for ( std::map<UINT32, std::vector<TraceRecordIndex> >::iterator it = tid2recordsmap.begin();
         it != tid2recordsmap.end();
         it++ )
   {
      rc = analysisRecordsByThread( it->first, cb, it->second, file, &flwFile, errFunctions, summaryRecords ) ;
      PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to analysisRecordsByThread, errno=%d", rc ) ;
   }

   //analysis result
   rc = dealWithExceptRecords( cb, file, &funcRecFile, &exceptFile, summaryRecords, tid2recordsmap ) ;
   PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to dealWithExceptRecords, errno=%d", rc ) ;

   //output
   outputErrorFunctions( errFunctions, &errFile ) ;

done :
   errFile.Close() ;
   exceptFile.Close() ;
   //smryFile.Close() ;
   flwFile.Close() ;
   return rc ;
error :
   goto done ;
}



// 分析指定线程的trace序列
// 1 输出该trace执行序列
// 2 分析trace中每个函数执行时间情况
// 3 记录异常trace记录
INT32 analysisRecordsByThread(UINT32 tid, 
                              pdTraceCB *cb,
                              std::vector<TraceRecordIndex> recIdxs, 
                              ossPrimitiveFileOp *file, 
                              ossPrimitiveFileOp *flwFile, 
                              std::map<UINT64, std::vector<UINT32> > &errFunctions, 
                              std::map<UINT64, FunctionSummaryRecord> &summaryRecords
                              )
{
  INT32 rc = SDB_OK ;
  CHAR tempBuf [ TRACE_RECORD_MAX_SIZE ] ;
  INT32 idx = 0 ;
  INT32 numIndent = 0 ;
  UINT64 cursor;
  UINT64 preTimestamp = 0, curTimeStamp ;
  UINT64 timeInterval = 0 ;
  pdTraceRecord *pRecord = NULL ;
  std::stack<FunctionRecord> funcStack ;

  rc = flwFile->fWrite( OSS_NEWLINE"tid: %u"OSS_NEWLINE, tid ) ;
  PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
            "Failed to write into trace file, errno = %d", rc ) ;
  for( ; idx < recIdxs.size(); idx++ )
  {
     cursor = recIdxs[idx]._offset;
     rc = readTraceRecord (file, cursor, cb->_headerSize, cb->_headerSize+cb->_totalSize, tempBuf ) ;
     if( rc ) goto error ;

     pRecord = (pdTraceRecord*)&tempBuf[0] ;

     curTimeStamp = pRecord->_timestamp.time * 1000000L + pRecord->_timestamp.microtm ;
     if (preTimestamp)
     {
        timeInterval = curTimeStamp - preTimestamp ;
     }
     //TODO: analysis function 
     analysisFunctionStack( funcStack, idx, recIdxs[idx]._sequenceNum, timeInterval, *pRecord, errFunctions, summaryRecords ) ;
     //todo: write to flw file
     rc = outputTraceRecordByThread( flwFile, recIdxs[idx]._sequenceNum, numIndent, timeInterval, tempBuf );
     PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                 "Failed to write into trace file, errno = %d", rc ) ;

     preTimestamp = curTimeStamp ;

  }

done :
   return rc ;
error :
   goto done ;
}

// 函数执行栈分析
// 1 分析函数的起止记录
// 2 统计函数当前层所执行所耗时间
// 3 记录当前层的最大时间间隔（峰值）（timeInterval）
// 4 获取trace记录异常点（trace记录不匹配）
// 5 统计函数当前层所含记录数
void analysisFunctionStack(std::stack<FunctionRecord> &funStack, 
                           UINT32 recdIndexIdx,
                           UINT32 sequenceNum,
                           UINT64 timeInterval,
                           pdTraceRecord curRecord, 
                           std::map<UINT64, std::vector<UINT32> > &errFunctions, 
                           std::map<UINT64, FunctionSummaryRecord> &summaryRecords)
{
   FunctionRecord popRecord, topRecord ;
   UINT64 timeStamp ;
   BOOLEAN isEmpty = funStack.empty() ;

   //
   if ( !isEmpty )
   {
      funStack.top()._maxTimeInterval = OSS_MAX( funStack.top()._maxTimeInterval, timeInterval ) ;
   }

   if ( curRecord._flag == PD_TRACE_RECORD_FLAG_ENTRY )
   {
      if ( !isEmpty )
         funStack.top()._nChild++ ;
      funStack.push( 
                     FunctionRecord(recdIndexIdx, 
                                    sequenceNum,
                                    curRecord._functionID, 
                                    curRecord._tid,
                                    0, 
                                    (INT64)(curRecord._timestamp.time * 1000000L + curRecord._timestamp.microtm),
                                    0,
                                    0,
                                    curRecord._timestamp) 
                    );
   }
   else if (  !isEmpty  && curRecord._flag == PD_TRACE_RECORD_FLAG_EXIT )
   {
      // step 1 pop
      popRecord = funStack.top();
      funStack.pop();

      // step 2 match function
      if ( popRecord._functionID == curRecord._functionID )
      {
         popRecord._cost = curRecord._timestamp.time * 1000000L + curRecord._timestamp.microtm - popRecord._cost ;
         popRecord._totalCost = curRecord._timestamp.time * 1000000L + curRecord._timestamp.microtm - 
                                   ( popRecord._start.time * 1000000L + popRecord._start.microtm );
         summaryRecords[popRecord._functionID].insert(popRecord);
         
         if ( !funStack.empty() )
         {
            /*funStack.top()._cost += curRecord._timestamp.time * 1000000L + curRecord._timestamp.microtm - 
                                   ( popRecord._start.time * 1000000L + popRecord._start.microtm );*/
            funStack.top()._cost += popRecord._totalCost ;
            funStack.top()._nChild++ ;
         }
      }
      else
      {
         //errFunctions.insert(popRecord._functionID) ;
         errFunctions[popRecord._functionID].push_back( sequenceNum ) ;
         std::stack<FunctionRecord> nullStack ;
         std::swap( funStack, nullStack ) ;
      }

   }
   else if( !isEmpty && curRecord._flag == PD_TRACE_RECORD_FLAG_NORMAL)
   {
      funStack.top()._nChild++ ;
   }

}

INT32 outputTraceRecordByFMT(ossPrimitiveFileOp *out, CHAR *tempBuf, UINT32 sequenceNum)
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( out, "out can't be NULL" ) ;
   CHAR timestamp[64] ;
   CHAR *pArgs ;
   pdTraceRecord *record;

   record = (pdTraceRecord*)tempBuf ;

   // write sequence
   rc = out->fWrite ( "%u: ", sequenceNum ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into trace file, errno = %d", rc ) ;
   // write function id and timestamp
   ossTimestampToString ( record->_timestamp, timestamp ) ;
   rc = out->fWrite ( "%s(%u): %s"OSS_NEWLINE, pdGetTraceFunction(record->_functionID), record->_line, timestamp ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into trace file, errno = %d", rc ) ;
   // write pid/tid/arguments
   rc = out->fWrite ( "tid: %u, numArgs: %u"OSS_NEWLINE, record->_tid, record->_numArgs ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into trace file, errno = %d", rc ) ;

   pArgs = &tempBuf[sizeof(pdTraceRecord)] ;
   for ( UINT32 i = 0; i < record->_numArgs; i++ )
   {
      // sanity check, make sure the argument pointer is within valid range
      if ( pArgs - &tempBuf[0] >= TRACE_RECORD_MAX_SIZE ||
         pArgs - &tempBuf[0] < (INT32)sizeof(pdTraceRecord) )
      {
         PD_RC_CHECK ( SDB_PD_TRACE_FILE_INVALID, PDERROR, "Invalid argument offset" ) ;
      }
      // assign the pointer to a temp argument structure
      pdTraceArgument *arg = (pdTraceArgument*)pArgs ;
      // move pArgs to next
      pArgs += arg->_argumentSize ;
      // write out the argument
      rc = out->fWrite ( "\targ%d:"OSS_NEWLINE, i ) ;
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into trace file, errno = %d", rc ) ;
      switch ( arg->_argumentType )
      {
       case PD_TRACE_ARGTYPE_NULL :
         rc = out->fWrite ( "\t\tNULL"OSS_NEWLINE ) ;
                            PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                            "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_CHAR :
         rc = out->fWrite ( "\t\t%c"OSS_NEWLINE, (*(CHAR*)(((CHAR*)arg)+ sizeof(pdTraceArgument))));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_BYTE :
         rc = out->fWrite ( "\t\t0x%x"OSS_NEWLINE,
                             (UINT32)(*(CHAR*)(((CHAR*)arg)+ sizeof(pdTraceArgument))));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                     "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_SHORT :
         rc = out->fWrite ( "\t\t%d"OSS_NEWLINE,
                            (INT32)(*(INT16*)(((CHAR*)arg)+
                            sizeof(pdTraceArgument))));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                     "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_USHORT :
         rc = out->fWrite ( "\t\t%u"OSS_NEWLINE,
                           (UINT32)(*(UINT16*)(((CHAR*)arg)+
                           sizeof(pdTraceArgument))));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                     "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_INT :
         rc = out->fWrite ( "\t\t%d"OSS_NEWLINE,
                           *(INT32*)(((CHAR*)arg)+sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                           "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_UINT :
         rc = out->fWrite ( "\t\t%u"OSS_NEWLINE,
                           *(UINT32*)(((CHAR*)arg)+sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                    "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_LONG :
         rc = out->fWrite ( "\t\t%lld"OSS_NEWLINE,
                            *(INT64*)(((CHAR*)arg)+sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                     "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_ULONG :
         rc = out->fWrite ( "\t\t%llu"OSS_NEWLINE,
                            *(UINT64*)(((CHAR*)arg)+sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                            "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_FLOAT :
         rc = out->fWrite ( "\t\t%f"OSS_NEWLINE,
                           *(FLOAT32*)(((CHAR*)arg)+
                           sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                    "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_DOUBLE :
         rc = out->fWrite ( "\t\t%f"OSS_NEWLINE,
                           *(FLOAT64*)(((CHAR*)arg)+
                           sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                  "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_STRING :
         rc = out->fWrite ( "\t\t%s"OSS_NEWLINE,
                           (((CHAR*)arg)+ sizeof(pdTraceArgument)));
         PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                     "Failed to write into trace file, errno = %d", rc ) ;
         break ;
       case PD_TRACE_ARGTYPE_RAW :
         {
            INT32 rawSize = arg->_argumentSize - sizeof(pdTraceArgument) ;
            UINT32 outSize = 10*rawSize ;
            CHAR *pTempBuffer = (CHAR*)SDB_OSS_MALLOC ( outSize ) ;
            PD_CHECK ( pTempBuffer, SDB_OOM, error, PDERROR,
                        "Failed to allocate memory for temp buffer" ) ;
            ossHexDumpBuffer ( (((CHAR*)arg)+
                                 sizeof(pdTraceArgument)),
                                 rawSize,
                                 pTempBuffer,
                                 outSize,
                                 NULL,
                                 OSS_HEXDUMP_INCLUDE_ADDR ) ;
            rc = out->fWrite ( "%s"OSS_NEWLINE, pTempBuffer ) ;
                               SDB_OSS_FREE ( pTempBuffer ) ;
            PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
                       "Failed to write into trace file, errno = %d", rc ) ;
            break ;
         }
       case PD_TRACE_ARGTYPE_NONE :
       default :
         break ;
      }
   }
   rc = out->fWrite ( OSS_NEWLINE ) ;
   PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR,
              "Failed to write into trace file, errno = %d", rc ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 outputTraceRecordByThread(ossPrimitiveFileOp *flwFile, UINT32 sequence, INT32 &numIndent, UINT64 timeInterval, CHAR *recordBuf)
{
   INT32 rc = SDB_OK ;
   CHAR timestamp[64] ;
   UINT64 curTimestamp ;
   pdTraceRecord *record = (pdTraceRecord *)recordBuf ;


   // write sequence
   rc = flwFile->fWrite ( OSS_NEWLINE"%6u:  ", sequence ) ;

   // for exit, let's decrease numIndent
   if ( record->_flag == PD_TRACE_RECORD_FLAG_EXIT )
   {
      numIndent -- ;
      if ( numIndent < 0 )  numIndent = 0 ;
   }
   // write indents
   for ( INT32 i = 0; i < numIndent; ++i )
   {
      rc = flwFile->Write ( "| ", 2 ) ;
      if ( rc ) goto error ;
   }

   // output the main part
   rc = flwFile->fWrite ( "%s",
      pdGetTraceFunction(record->_functionID),
      record->_line ) ;
   if ( rc ) goto error ;

   // then check if it's start/exit
   if ( record->_flag == PD_TRACE_RECORD_FLAG_ENTRY )
   {
      rc = flwFile->fWrite ( " Entry" ) ;
      if ( rc ) goto error ;
      numIndent ++ ;
   }
   else if ( record->_flag == PD_TRACE_RECORD_FLAG_EXIT )
   {
      rc = flwFile->fWrite ( " Exit" ) ;
      if ( rc ) goto error ;
      // exit with rc
      if ( record->_numArgs == 1 )
      {
         pdTraceArgument *arg =
            (pdTraceArgument*)&recordBuf[sizeof(pdTraceRecord)] ;
         if ( PD_TRACE_ARGTYPE_INT == arg->_argumentType )
         {
            INT32 retCode = *(INT32*)(((CHAR*)arg)+sizeof(pdTraceArgument));
            // show return code only when it's not SDB_OK
            if ( SDB_OK != retCode )
            {
               rc = flwFile->fWrite ( "[retCode=%d]", retCode ) ;
               if ( rc ) goto error ;
            }
         } // if ( PD_TRACE_ARGTYPE_INT == arg->_argumentType )
      } // if ( record->_numArgs == 1 )
   } // else if ( record->_flag == PD_TRACE_RECORD_FLAG_EXIT )
   // finally write function name and timestamp
   ossTimestampToString ( record->_timestamp, timestamp ) ;
   rc = flwFile->fWrite ( "(%u): %s", record->_line, timestamp ) ;

   if ( timeInterval > TRACE_RECORD_EXCEPTION_TIME_THRESHOLD )
   {
      rc = flwFile->fWrite ( "  (%ld)", timeInterval ) ;
   }
done :
   return rc ;
error :
   goto done ;
}

INT32 outputFunctionSummaryRecord(UINT64 funcId, FunctionSummaryRecord &record, ossPrimitiveFileOp *funcFile)
{
   INT32 rc = SDB_OK ;

   // name, count, avgcost, min, first, second, third, fourth, fifth
   rc = funcFile->fWrite ( OSS_NEWLINE"%s,%d,%.1f,%u", pdGetTraceFunction(funcId), record._count, record._avgcost, record._minCost ) ;
   int count = OSS_MIN ( record._count, NUMBER_OF_FUNCTION_RECORD_RESERVATION ) ;
   for (int ifunc = 0; ifunc < count && (SDB_OK == rc); ifunc++)
   {
      rc = funcFile->fWrite ( ",\"(%u, %ld, %ld, %ld)\"", record._reserveRecords[ifunc]._sequenceNum, 
                                                               record._reserveRecords[ifunc]._totalCost,
                                                               record._reserveRecords[ifunc]._cost, 
                                                               record._reserveRecords[ifunc]._maxTimeInterval  ) ;
      //funcFile->fWrite ( "   tid:%u \n", it->second._reserveRecords[ifunc]._tid ) ;
   }

   return rc ;
}


INT32 selectExceptRecords(FunctionSummaryRecord &record, std::set<FunctionRecord> &exceptRecords)
{
   INT32 rc = SDB_OK ;
   if (record._count && record._reserveRecords[0]._maxTimeInterval > 3 * TRACE_RECORD_EXCEPTION_TIME_THRESHOLD)
   {
      //insertFixedArray( it->second._reserveRecords[0], recordArray, count, MAX_LENGTH_EXCEPTION_RECORD ) ;
      exceptRecords.insert(record._reserveRecords[0]) ;
   }

   return rc;
}

INT32 dealWithExceptRecords(pdTraceCB *cb, 
                            ossPrimitiveFileOp *dumpFile, 
                            ossPrimitiveFileOp *funcRecFile,
                            ossPrimitiveFileOp *exceptFile, 
                            std::map<UINT64, FunctionSummaryRecord> &summaryRecords, 
                            std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap)
{
   INT32 rc = SDB_OK ;
   std::set<FunctionRecord> exceptRecords ;
   rc = funcRecFile->fWrite("name, count, avgcost, min, first, second, third, fourth, fifth") ;
   PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to write to file, errno=%d", rc ) ;

   for (std::map<UINT64, FunctionSummaryRecord>::iterator it = summaryRecords.begin(); it != summaryRecords.end(); it++)
   {
      rc = selectExceptRecords( it->second, exceptRecords ) ;
      PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to selectExceptRecords, errno=%d", rc ) ;

      rc = outputFunctionSummaryRecord( it->first, it->second, funcRecFile ) ;
      PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to outputFunctionSummaryRecord, errno=%d", rc ) ;
   }

   rc = outputExceptionReport( cb, dumpFile, exceptFile, exceptRecords, tid2recordsmap ) ;
   PD_CHECK ( 0 == rc, rc, error, PDERROR, "Failed to outputExceptionReport, errno=%d", rc ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 outputTraceRecord(ossPrimitiveFileOp *flwFile, pdTraceRecord &record, UINT32 sequence, BOOLEAN isChild)
{
   INT32 rc = SDB_OK ;
   CHAR timestamp[64] ;
   UINT64 curTimestamp ;

   // write sequence
   rc = flwFile->fWrite ( OSS_NEWLINE"%6u:  ", sequence ) ;
   // for exit, let's decrease numIndent

   if ( isChild ) rc = flwFile->Write ( "| ", 2 ) ;

   // output the main part
   rc = flwFile->fWrite ( "%s", pdGetTraceFunction(record._functionID), record._line ) ;
   if ( rc ) goto error ;

   // then check if it's start/exit
   if ( record._flag == PD_TRACE_RECORD_FLAG_ENTRY )
   {
      rc = flwFile->fWrite ( " Entry" ) ;
      if ( rc ) goto error ;
   }
   else if ( record._flag == PD_TRACE_RECORD_FLAG_EXIT )
   {
      rc = flwFile->fWrite ( " Exit" ) ;
      if ( rc ) goto error ;
      // exit with rc
   } // else if ( record->_flag == PD_TRACE_RECORD_FLAG_EXIT )
   // finally write function name and timestamp
   ossTimestampToString ( record._timestamp, timestamp ) ;
   rc = flwFile->fWrite ( "(%u): %s", record._line, timestamp ) ;
done :
   return rc ;
error :
   goto done ;
}


// 输出异常记录片段
// 1、值输出异常所在层执行序列
// 2 片段行数不满40，全部输出，超过40时的只输出异常点
INT32  outputExceptionReport(pdTraceCB *cb,
                          ossPrimitiveFileOp *dumpFile, 
                          ossPrimitiveFileOp *reportFile, 
                          std::set<FunctionRecord> &exceptRecords,
                          std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap)
{
   INT32 rc = SDB_OK ;
   UINT32 tid;
   UINT64 cursor;
   INT32  indent ;
   INT32  nScan ;
   BOOLEAN isChild ;
   BOOLEAN isFinish ;
   BOOLEAN isPrintAll = TRUE ;
   CHAR tempBuf [ TRACE_RECORD_MAX_SIZE ] ;
   pdTraceRecord *pRecord = NULL ;
   pdTraceRecord preRecord ;
   UINT64 preTimestamp, curTimestap, timeInterval ;


   for (std::set<FunctionRecord>::reverse_iterator rit = exceptRecords.rbegin(); rit != exceptRecords.rend(); rit++)
   {
      isPrintAll = TRUE ;
      indent = 0 ;
      nScan = 0 ;
      isFinish = FALSE ;
      preTimestamp = 0 ;
      tid = rit->_tid ;

      rc = reportFile->fWrite( OSS_NEWLINE"-------------------------------------------------------------------------"OSS_NEWLINE);
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;


      rc = reportFile->fWrite( "%s "OSS_NEWLINE, pdGetTraceFunction( rit->_functionID ) ) ;
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;

      rc = reportFile->fWrite( "sequence: %u tid: %u  cost: %ld (%ld) "OSS_NEWLINE, rit->_sequenceNum, 
                                                                 tid, rit->_cost, rit->_maxTimeInterval) ;
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;

      if (rit->_nChild > MAX_LENGTH_PRINT_LINE_EXCEPTION_RECORD)
      {
         isPrintAll = FALSE ;
      }

      for (int idx = rit->_indexidx; idx < tid2recordsmap[tid].size() && !isFinish; idx++)
      {
         timeInterval = 0 ; 
         isChild = TRUE ;

         cursor = tid2recordsmap[tid][idx]._offset;
         rc = readTraceRecord (dumpFile, cursor, cb->_headerSize, cb->_headerSize+cb->_totalSize, tempBuf ) ;
         if( rc ) break ;

         pRecord = (pdTraceRecord*)&tempBuf[0] ;
         //todo: write to flw file

         if (pRecord->_flag == PD_TRACE_RECORD_FLAG_EXIT)
         {
            indent--;
            if( indent > 1 ) continue ;
            if( indent == 0 ) isFinish = TRUE ;
         }
         else if (pRecord->_flag == PD_TRACE_RECORD_FLAG_ENTRY)
         {
            indent++;
            if( indent > 2 ) continue ;
         }
         else 
         {
            if( indent > 1 ) continue ;
         }
         curTimestap = pRecord->_timestamp.time * 1000000L + pRecord->_timestamp.microtm ;
         if (preTimestamp && !(pRecord->_flag == PD_TRACE_RECORD_FLAG_EXIT && indent > 0))
         {
            timeInterval = curTimestap - preTimestamp ;
         }

         if ( !preTimestamp || isFinish)
         {
            isChild = FALSE ;
         }
         
         if (isPrintAll)
         {
            rc = outputTraceRecord(reportFile, *pRecord, tid2recordsmap[tid][idx]._sequenceNum, isChild);
            if (timeInterval)
            {
               rc = reportFile->fWrite ( "      (%ld)", timeInterval ) ;
               PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;
            }
         }
         else
         {
            if ( !preTimestamp  )
            {
               rc = outputTraceRecord(reportFile, *pRecord, tid2recordsmap[tid][idx]._sequenceNum, isChild);
            }
            else if ( isFinish || timeInterval > TRACE_RECORD_EXCEPTION_TIME_THRESHOLD )
            {
               if( nScan > 2 )  
               {
                  rc = reportFile->fWrite ( OSS_NEWLINE"           . . . .  " ) ;
                  PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;
               }
               if( nScan > 1 ) rc = outputTraceRecord(reportFile, preRecord, tid2recordsmap[tid][idx-1]._sequenceNum, TRUE);
               rc = outputTraceRecord(reportFile, *pRecord, tid2recordsmap[tid][idx]._sequenceNum, isChild);
               if (timeInterval)
               {
                  rc = reportFile->fWrite ( "      (%ld)", timeInterval ) ;
                  PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;
               }
               nScan = 0 ;
            }
            preRecord = *pRecord ;
            nScan++ ;
         }

         preTimestamp = curTimestap ;
      }
      rc = reportFile->fWrite( OSS_NEWLINE );
      PD_CHECK ( 0 == rc, SDB_IO, error, PDERROR, "Failed to write into except file, errno = %d", rc ) ;

   }

done :
   return rc ;
error :
   goto done ;
}


