/************************************
*@description : Public function for explain
*@author : chimanzhao
*@createdate:  2020.5.6
**************************************/
import( "../lib/basic_operation/sequoiadb.js" );
import( "../lib/main.js" );

function testExplain ( conds, dbcl, indexName, scanType )
{
   for( var i = 0; i < conds.length; ++i )
   {
      checkExplain( dbcl, conds[i], indexName, scanType );
   }
}

function checkExplain ( dbcl, cond, expIndexName, expScanType )
{
   var explainObj = dbcl.find( cond ).explain().next().toObj();
   var IndexName = explainObj.IndexName;
   var ScanType = explainObj.ScanType;
   if( expIndexName !== IndexName || expScanType !== ScanType )
   {
      throw new Error( "索引选择错误！" )
   }
}

/**
 * 检查最优索引选择是否与期望值相符合
 * @param  cursor 执行查询计划得到的 cursor
 * @param  expIndex 期望索引
 */
function checkOptimalIndex ( cursor, expIndex )
{
   var searchPaths = cursor.current().toObj().PlanPath.ChildOperators[0].Search.SearchPaths;
   for( var i = 0; i < searchPaths.length; i++ )
   {
      var actIndex = searchPaths[i].IndexName;
      var isUsed = searchPaths[i].IsUsed;
      if( isUsed === true )
      {
         if( expIndex !== actIndex )
         {
            throw new Error( "optimal Index error,expIndex:" + expIndex + ",but actually actIndex:" + actIndex );
         }
      }
   }
}
/**
 * 检查是否将除表扫描外的索引加入候选计划
 * @param  cursor 执行查询计划得到的 cursor
 * @param  expCandidates 期望的候选计划
 */
function checkCandidates ( cursor, expCandidates )
{
   var searchPaths = cursor.current().toObj().PlanPath.ChildOperators[0].Search.SearchPaths;
   var actCandidates = [];
   for( var i = 0; i < searchPaths.length; i++ )
   {
      var isCandidate = searchPaths[i].IsCandidate;
      var IndexName = searchPaths[i].IndexName;
      if( isCandidate === true && IndexName !== "" )
      {
         actCandidates.push( IndexName );

      }
   }
   if( !commCompareObject( expCandidates, actCandidates ) )
   {
      throw new Error( "expCandidates:" + expCandidates + ",but actCandidates:" + actCandidates );
   }
}