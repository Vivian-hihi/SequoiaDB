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
   var idxName1 = "index_ab_11374";
   var idxName2 = "index_ba_11374";
   var tbIdx = "";

   commDropCL( db, COMMCSNAME, clName );

   var cl = commCreateCL( db, COMMCSNAME, clName, { Group: dataGroupNames[0] }, false );

   cl.createIndex( idxName1, { a: 1, b: -1 } );
   cl.createIndex( idxName2, { a: -1, b: 1 } );

   // 生成随机数
   var rd = new commDataGenerator();
   var value = rd.getRecords( 1000, "int", ["a", "b", "c"] );
   cl.insert( value );

   db.analyze();

   var cond = { "a": { "$gt": 1 } };
   var expIndexName = tbIdx;
   var expScanType = "tbscan";
   var sortCond = { "a": 1, "b": -1, "c": 1 };
   checkExplain( cl, cond, expIndexName, expScanType, sortCond );


   commDropCL( db, COMMCSNAME, clName, false );
}