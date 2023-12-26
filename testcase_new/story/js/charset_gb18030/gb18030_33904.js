/************************************
*@Description: seqDB-33904 设置字符集为GB18030，执行索引操作
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "数据库_33904";
   var clName = "集合_33904";
   var indexName = "索引_33904";
   var indexKeyName = "索引键_33904";

   commDropCL( db, csName, clName );
   var cl = commCreateCL( db, csName, clName );

   // create index
   var indexDef = {};
   indexDef[indexKeyName] = 1;
   cl.createIndex( indexName, indexDef );
   var indexInfo = cl.getIndex( indexName ).toObj();
   assert.equal( indexInfo.IndexDef.name, indexName );
   assert.equal( indexInfo.IndexDef.key, indexDef );

   // insert data
   var obj1 = {};
   var obj2 = {};
   var obj3 = {};
   var obj4 = {};
   var obj5 = {};
   var obj6 = {};
   obj1[indexKeyName] = "测试数据";
   obj2[indexKeyName] = "abc";
   obj3[indexKeyName] = "123";
   obj4[indexKeyName] = " ";
   obj5[indexKeyName] = "爯爲𫌀𫖳䰲䰳";
   obj6[indexKeyName] = "&*%$#@!";
   var data = [obj1, obj2, obj3, obj4, obj5, obj6];
   cl.insert( data );

   // select by index
   var cursor = cl.find().hint( { "": indexName } );
   var expected = [obj4, obj6, obj3, obj2, obj1, obj5];
   commCompareResults( cursor, expected );

   // drop index
   commDropIndex( cl, indexName, true );
   assert.tryThrow( SDB_IXM_NOTEXIST, function()
   {
      cl.getIndex( indexName );
   } );
   commDropCS( db, csName );
}