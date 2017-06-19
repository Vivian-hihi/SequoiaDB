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

   Source File Name = pdTraceAnalysis.hpp

   Descriptive Name = define the operation for analysis trace

   When/how to use: this program may be used to analyze trace file

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who            Description
   ====== =========== ===            ==============================================
          06/16/2017  Huangansheng   Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PDTRACEANALYSIS_HPP__
#define PDTRACEANALYSIS_HPP__


#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "ossAtomic.hpp"
#include "pdTrace.hpp"
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#ifdef SDB_ENGINE
#include "pdTrace.h"
#include "ossLatch.hpp"
#endif
#include "ossPrimitiveFileOp.hpp"


#define  MAX_LENGTH_EXCEPTION_RECORD 10
#define  MAX_LENGTH_PRINT_LINE_EXCEPTION_RECORD 40
#define  TRACE_RECORD_EXCEPTION_TIME_THRESHOLD 1000

struct TraceRecordIndex
{
   UINT32  _sequenceNum;
   UINT64  _offset;

   TraceRecordIndex(){}
   TraceRecordIndex(UINT32 sequenceNum, UINT64 offset): _sequenceNum(sequenceNum),_offset(offset){}
};


struct FunctionRecord
{
   UINT32        _indexidx ;            
   UINT32        _sequenceNum ;
   UINT32        _tid ;
   UINT32        _nChild ;
   UINT64        _cost ;           
   UINT64        _totalCost ;       
   UINT64        _maxTimeInterval ;
   UINT64        _functionID ;
   ossTimestamp  _start ;

   FunctionRecord(){}
   FunctionRecord( UINT32 indexidx, 
                   UINT32 sequence, 
                   UINT32 tid, 
                   UINT32 nchild, 
                   UINT64 cost,
                   UINT64 totalCost,
                   UINT64  maxTimeInterval,
                   UINT64 funcID, 
                   ossTimestamp starttime): 
                     _indexidx(indexidx), 
                     _sequenceNum(sequence), 
                     _tid(tid), 
                     _nChild(nchild), 
                     _cost(cost), 
                     _totalCost(totalCost),
                     _maxTimeInterval(maxTimeInterval),
                     _functionID(funcID), 
                     _start(starttime){}

   bool operator < (const FunctionRecord &another) const
   {
      return _maxTimeInterval < another._maxTimeInterval ;
   }

  
} ;


#define NUMBER_OF_FUNCTION_RECORD_RESERVATION 5
struct FunctionSummaryRecord
{
   UINT32          _count ;
   UINT32          _minCost ;
   double          _avgcost ;
   FunctionRecord  _reserveRecords[NUMBER_OF_FUNCTION_RECORD_RESERVATION] ; 

   FunctionSummaryRecord(): _count(0), _avgcost(0){}
   void insert(FunctionRecord record)
   {
      INT8 idx = OSS_MIN ( _count, NUMBER_OF_FUNCTION_RECORD_RESERVATION ) ;  
      for ( idx = idx - 1; idx >= 0; idx-- )
      {
         if (record._maxTimeInterval <= _reserveRecords[idx]._maxTimeInterval) break; 
         if ( idx != NUMBER_OF_FUNCTION_RECORD_RESERVATION-1 )
         {
            _reserveRecords[idx+1] = _reserveRecords[idx] ;
         }
      }

      //inset idx+1
      if (idx + 1 < NUMBER_OF_FUNCTION_RECORD_RESERVATION)
      {
         _reserveRecords[idx+1] = record ;
      }


      if ( _count == 0 )
      {
         _minCost = (UINT32)record._cost ;
      }
      else
      {
         _minCost = OSS_MIN ( (UINT32)record._cost, _minCost ) ;
      }

      _count++;
      _avgcost = ((_count-1)*_avgcost + (double)record._cost)/_count ;

   }
};


// step1: output fmt file
// step2: analyze the record index
INT32 parseTraceDumpFile(ossPrimitiveFileOp *file, 
                         pdTraceCB *cb,
                         CHAR *fmtFilePath,
                         std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap
                         );


INT32 analysisTraceRecords( ossPrimitiveFileOp *file, 
                            pdTraceCB *cb,
                            std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap,
                            CHAR *errorFilePath, 
                            CHAR *funcRecordPath, 
                            CHAR *flwFilePath, 
                            CHAR *summaryFilePath, 
                            CHAR *exceptFilePath );

// 1 output program execution sequence
// 2 calculate function execution time
// 3 analyze exception record
INT32 analysisRecordsByThread(   UINT32 tid,
                                 pdTraceCB *cb,
                                 std::vector<TraceRecordIndex> recIdxs,
                                 ossPrimitiveFileOp *file,
                                 ossPrimitiveFileOp *flwFile,
                                 std::map<UINT64, std::vector<UINT32> > &errFunctions, 
                                 std::map<UINT64, FunctionSummaryRecord> &summaryRecords);



// 1 calculate function execution time
// 2 maxtimeInterval
// 3 handing error records
void  analysisFunctionStack(std::stack<FunctionRecord> &funStack,
                            UINT32 recdIndexIdx,
                            UINT32 sequenceNum,
                            UINT64 timeInterval,
                            pdTraceRecord curRecord,
                            std::map<UINT64, std::vector<UINT32> > &errFunctions, 
                            std::map<UINT64, FunctionSummaryRecord> &summaryRecords);


// 1 select exception 
// 2 output exception 
INT32 dealWithExceptRecords(pdTraceCB *cb,
                            ossPrimitiveFileOp *dumpFile, 
                            ossPrimitiveFileOp *funcRecFile,
                            ossPrimitiveFileOp *exceptFile, 
                            std::map<UINT64, FunctionSummaryRecord> &summaryRecords,
                            std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap) ;



// 



#endif // PDTRACEANALYSIS_HPP__
