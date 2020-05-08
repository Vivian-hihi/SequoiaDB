/******************************************************************************
*@Description : seqDB-11372:匹配组合索引的索引选择
*@author      : Li Yuanyue
*@Date        : 2020.4.25
******************************************************************************/
testConf.clName = CHANGEDPREFIX + "_11372";

main( test );

function test ()
{
   var dataGroupNames = commGetDataGroupNames( db );
   var clName = CHANGEDPREFIX + "_11372";
   var idxName1 = "index_ab_11372";
   var tbIdx = "";

   commDropCL( db, COMMCSNAME, clName );

   var cl = commCreateCL( db, COMMCSNAME, clName, { Group: dataGroupNames[0] }, false );

   cl.createIndex( idxName1, { a: 1, b: 1, c: 1 } );

   var docs = [];
   for( var i = 0; i < 30000; i++ )
   {
      docs.push( { a: 1, b: 1, c: -1 } );
   }
   cl.insert( docs );

   // before
   var cond = [{ "a": 1, "b": 1, "c": -1 }, { "a": 1, "c": -1 }];
   var expIndexName = idxName1;
   var expScanType = "ixscan";
   testExplain( cl, cond, expIndexName, expScanType );

   db.analyze();

   // after
   var cond = [{ "a": 1, "b": 1, "c": -1 }, { "a": 1, "c": -1 }];
   var expIndexName = tbIdx;
   var expScanType = "tbscan";
   testExplain( cl, cond, expIndexName, expScanType );

   commDropCL( db, COMMCSNAME, clName, false );
}