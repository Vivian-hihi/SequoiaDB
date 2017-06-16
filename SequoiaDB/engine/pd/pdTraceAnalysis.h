#ifndef pdTraceAnalysis_h__
#define pdTraceAnalysis_h__

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

//函数记录结构
struct FunctionRecord
{
   UINT32        _indexidx ;             //输出片段使用
   UINT32        _sequenceNum ;
   UINT32        _tid ;
   UINT32        _nChild ;
   UINT64        _cost ;            //当前消耗，不包含子函数
   UINT64        _totalCost ;       
   UINT64        _maxTimeInterval ;
   UINT64        _functionID ;
   ossTimestamp  _start ;

   FunctionRecord(){}
   FunctionRecord( UINT32 indexidx, 
                   UINT32 sequence, 
                   UINT64 funcID, 
                   UINT32 tid, 
                   UINT32 nchild, 
                   UINT64 cost,
                   UINT64 totalCost,
                   UINT64  maxTimeInterval,
                   ossTimestamp starttime): 
                     _indexidx(indexidx), 
                     _sequenceNum(sequence), 
                     _functionID(funcID), 
                     _tid(tid), 
                     _nChild(nchild), 
                     _cost(cost), 
                     _totalCost(totalCost),
                     _maxTimeInterval(maxTimeInterval),
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
   FunctionRecord  _reserveRecords[NUMBER_OF_FUNCTION_RECORD_RESERVATION] ; //记录保留的最长间隔的记录

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




// 分析trace序列，并输出到相应的文件中
// 1）	输出每个线程的记录索引信息
// 2）	输出fmt文件
INT32 parseTraceDumpFile(ossPrimitiveFileOp *file, 
                         pdTraceCB *cb,
                         CHAR *fmtFilePath,
                         std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap
                         );

// 分析trace序列，并输出到相应的文件中
INT32 analysisTraceRecords(ossPrimitiveFileOp *file, 
                      pdTraceCB *cb,
                      std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap,
                      CHAR *errorFilePath, 
                      CHAR *funcRecordPath, 
                      CHAR *flwFilePath, 
                      CHAR *summaryFilePath, 
                      CHAR *exceptFilePath);

// 分析指定线程的trace序列
// 1 输出该trace执行序列
// 2 分析trace中每个函数执行时间情况
// 3 记录异常trace记录
INT32 analysisRecordsByThread( UINT32 tid,
                     pdTraceCB *cb,
                     std::vector<TraceRecordIndex> recIdxs,
                     ossPrimitiveFileOp *file,
                     ossPrimitiveFileOp *flwFile,
                     std::map<UINT64, std::vector<UINT32> > &errFunctions, 
                     std::map<UINT64, FunctionSummaryRecord> &summaryRecords);



// 函数执行栈分析
// 1 分析函数的起止记录
// 2 统计函数当前层所执行所耗时间
// 3 记录当前层的最大时间间隔（峰值）（timeInterval）
// 4 获取trace记录异常点（trace记录不匹配）
// 5 统计函数当前层所含记录数
void  analysisFunctionStack(std::stack<FunctionRecord> &funStack,
                            UINT32 recdIndexIdx,
                            UINT32 sequenceNum,
                            UINT64 timeInterval,
                            pdTraceRecord curRecord,
                            std::map<UINT64, std::vector<UINT32> > &errFunctions, 
                            std::map<UINT64, FunctionSummaryRecord> &summaryRecords);


// 异常记录片段处理
// 1 选取异常片段记录
// 2 输出异常片段，片段行数不满40，全部输出，超过40时的只输出异常点
INT32 dealWithExceptRecords(pdTraceCB *cb,
                            ossPrimitiveFileOp *dumpFile, 
                            ossPrimitiveFileOp *funcRecFile,
                            ossPrimitiveFileOp *exceptFile, 
                            std::map<UINT64, FunctionSummaryRecord> &summaryRecords,
                            std::map<UINT32, std::vector<TraceRecordIndex> > &tid2recordsmap) ;



// 



#endif // pdTraceAnalysis_h__
