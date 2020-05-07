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
   var cursor = cl.find( { "a": 1, "b": 1, "c": -1 } ).explain( { Evaluate: true } );
   var expIndex = idxName1;
   checkOptimalIndex( cursor, expIndex );

   var cursor = cl.find( { "a": 1, "c": -1 } ).explain( { Evaluate: true } );
   var expIndex = idxName1;
   checkOptimalIndex( cursor, expIndex );

   db.analyze();

   // after
   var cursor = cl.find( { "a": 1, "b": 1, "c": -1 } ).explain( { Evaluate: true } );
   var expIndex = tbIdx;
   checkOptimalIndex( cursor, expIndex );

   var cursor = cl.find( { "a": 1, "c": -1 } ).explain( { Evaluate: true } );
   var expIndex = tbIdx;
   checkOptimalIndex( cursor, expIndex );

   commDropCL( db, COMMCSNAME, clName, false );
}