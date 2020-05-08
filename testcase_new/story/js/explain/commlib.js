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