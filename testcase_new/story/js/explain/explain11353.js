/************************************
*@Description: seqDB-11353:seqDB-11353:rtnPredicate为[$minKey, $maxKey]的索引选择
*@author:      chimanzhao
*@createdate:  2020.4.25
*@testlinkCase: seqDB-11353
**************************************/
main( test );

function test ()
{
   var clName = COMMCLNAME + "_11353";
   commDropCL( db, COMMCSNAME, clName );
   var dbcl   = commCreateCL( db, COMMCSNAME, clName );
   dbcl.createIndex( "a", {a:1} );
 
   //使用非索引字段进行查询的查询条件
   cond_non = { b: 1 };
   //使用or进行查询的查询条件
   cond_or = { $or:[{a:1}, {c:1}] };
   //使用not进行查询的查询条件
   cond_not = { $not:[{a:1}, {c:1}] };

   //不计算IO代价
   var docs=[];
   for (var i = 0; i < 1000; i++ )
   {
      docs.push( { a:i } )
   }
   dbcl.insert( docs );
   
   for(var i  = 0; i <5; i++)
   {
      dbcl.update( { $inc:{ b:i } }, { a:i } );
      dbcl.update( { $inc:{ c:-i } }, { a:i } )
   }
   testExplain( dbcl, cond_non, "", "tbscan" );
   testExplain( dbcl, cond_or, "", "tbscan" );
   testExplain( dbcl, cond_not, "", "tbscan" );
   db.analyze()
   testExplain( dbcl, cond_non, "", "tbscan" );
   testExplain( dbcl, cond_or, "", "tbscan" );
   testExplain( dbcl, cond_not, "", "tbscan" );
   
   //计算IO代价
   //添加数据使数据页数大于optestcachesize（20）
   var docs=[];
   for (var i = 0; i < 50000; i++ )
   {
      docs.push( { d:i } )
   }
   dbcl.insert( docs );

   testExplain( dbcl, cond_non, "", "tbscan" );
   testExplain( dbcl, cond_or, "", "tbscan" );
   testExplain( dbcl, cond_not, "", "tbscan" );
   db.analyze()
   testExplain( dbcl, cond_non, "", "tbscan" );
   testExplain( dbcl, cond_or, "", "tbscan" );
   testExplain( dbcl, cond_not, "", "tbscan" );
  
   commDropCL( db, COMMCSNAME, clName );
}

function testExplain( dbcl, cond, expIndexName, expScanType )
{
   var explainObj = dbcl.find( cond ).explain().next().toObj();
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType;
   if(expIndexName!==IndexName || expScanType!==ScanType)
   {
      throw new Error("索引选择错误！")
   }
}