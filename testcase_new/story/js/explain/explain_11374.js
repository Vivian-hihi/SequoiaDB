/******************************************************************************
*@Description : seqDB-11374:查询并排序，且排序为内排的索引选择
*@author      : Li Yuanyue
*@Date        : 2020.5.6
******************************************************************************/
testConf.clName = CHANGEDPREFIX + "_11374";

main( test );

function test ()
{
   var dataGroupNames = commGetDataGroupNames( db );
   var clName = CHANGEDPREFIX + "_11374";
   var idxName1 = "index_abc_11374";

   commDropCL( db, COMMCSNAME, clName );

   var cl = commCreateCL( db, COMMCSNAME, clName, { Group: dataGroupNames[0] }, false );

   cl.createIndex( idxName1, { a: 1, b: -1 } );

   // 生成随机数
   var rd = new commDataGenerator();
   var value = rd.getRecords( 3000, "int", ["a", "b", "c"] );
   cl.insert( value );

   db.analyze();

   // 验证是否加入候选计划
   var cursor = cl.find( { "a": 1, "b": 1 } ).sort( { "a": 1, "b": 1, "c": -1 } ).explain( { Evaluate: true } );
   var expCandidates = [idxName1];
   checkCandidates( cursor, expCandidates );

   // 验证是否选择最优索引
   var cursor = cl.find( { "a": 1, "b": 1, "c": 1 } ).sort( { "a": 1, "b": 1, "c": -1 } ).explain( { Evaluate: true } );
   var expIndex = idxName1;
   checkOptimalIndex( cursor, expIndex );

   commDropCL( db, COMMCSNAME, clName, false );
}