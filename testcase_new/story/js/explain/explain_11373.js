/******************************************************************************
*@Description : seqDB-11373:查询带排序，前一阶段查询结果有序的索引选择 
*@author      : Li Yuanyue
*@Date        : 2020.4.27
******************************************************************************/
testConf.clName = CHANGEDPREFIX + "_11373";

main( test );

function test ()
{
   var dataGroupNames = commGetDataGroupNames( db );
   var clName = CHANGEDPREFIX + "_11373";
   var idxName1 = "index_abc_11373";
   var idxName2 = "index_ab_11373";
   var idxName3 = "index_a_11373";

   commDropCL( db, COMMCSNAME, clName );

   var cl = commCreateCL( db, COMMCSNAME, clName, { Group: dataGroupNames[0] }, false );

   cl.createIndex( idxName1, { a: 1, b: 1, c: 1 } );
   cl.createIndex( idxName2, { a: 1, b: -1 } );
   cl.createIndex( idxName3, { a: 1 } );

   // 生成随机数
   var rd = new commDataGenerator();
   var value = rd.getRecords( 3000, "int", ["a", "b", "c"] );
   cl.insert( value );

   db.analyze();

   // 验证是否加入候选计划
   var cursor = cl.find( { "a": 1, "b": 1 } ).sort( { "a": 1 } ).explain( { Evaluate: true } );
   var expCandidates = [idxName1, idxName2, idxName3];
   checkCandidates( cursor, expCandidates );

   // 验证是否选择最优索引
   var cursor = cl.find( { "a": 1, "b": 1, "c": 1 } ).sort( { "a": 1 } ).explain( { Evaluate: true } );
   var expIndex = idxName1;
   checkOptimalIndex( cursor, expIndex );

   commDropCL( db, COMMCSNAME, clName, false );
}