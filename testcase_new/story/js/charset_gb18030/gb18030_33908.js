/************************************
*@Description: seqDB-33908 设置字符集为GB18030，执行集合挂载
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/
testConf.skipStandAlone = true;
main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "集合空间_33908";
   var mclName = "主集合_33908";
   var sclName1 = "子集合_33908_1";
   var sclName2 = "子集合_33908_2";
   var sclFullName1 = csName + "." + sclName1;
   var sclFullName2 = csName + "." + sclName2;
   commDropCL( db, csName, mclName );
   commDropCL( db, csName, sclName1 );
   commDropCL( db, csName, sclName2 );

   var recsNum = 100;
   var docs = [];
   for( var i = 0; i < recsNum; i++ )
   {
      docs.push( { "a": "测试数据" + i, "b": i } );
   }

   // create main cl
   var options = { "IsMainCL": true, "ShardingKey": { "a": 1 }, "ShardingType": "range" };
   var mcl = commCreateCL( db, csName, mclName, options );
   var scl1 = commCreateCL( db, csName, sclName1 );
   var scl2 = commCreateCL( db, csName, sclName2 );

   // attach sub cl
   mcl.attachCL( sclFullName1, { LowBound: { a: MinKey() }, UpBound: { a: "测试数据50" } } );
   mcl.attachCL( sclFullName2, { LowBound: { a: "测试数据50" }, UpBound: { a: MaxKey() } } );

   // insert data
   mcl.insert( docs );

   // check data distribution
   var expectCountScl1 = Number( mcl.find( { "a": { "$lt": "测试数据50" } } ).count() );
   var expectCountScl2 = Number( mcl.find( { "a": { "$gte": "测试数据50" } } ).count() );
   var actualCountScl1 = Number( scl1.find().count() );
   var actualCountScl2 = Number( scl2.find().count() );
   assert.equal( expectCountScl1, actualCountScl1, "expectCountScl1 is " + expectCountScl1 + ", actualCountScl1 is " + actualCountScl1 );
   assert.equal( expectCountScl2, actualCountScl2, "expectCountScl2 is " + expectCountScl2 + ", actualCountScl2 is " + actualCountScl2 );

   commDropCS( db, csName );
}