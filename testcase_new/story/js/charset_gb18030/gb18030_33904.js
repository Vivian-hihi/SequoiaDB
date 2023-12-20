/************************************
*@Description: 设置字符集为GB18030，执行索引操作
*@author:      chenzejia
*@createdate:  2023.12.16
*@testlinkCase:seqDB-33904
**************************************/

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "数据库_33904";
   var clName = "集合_33904";
   var indexName = "索引_33904";

   var cl = commCreateCL( db, csName, clName );

   // create index
   cl.createIndex( indexName, { "a": 1 } );
   var indexInfo = cl.getIndex( indexName ).toObj();
   assert.equal( indexInfo.IndexDef.name, indexName );
   assert.equal( indexInfo.IndexDef.key, { "a": 1 } );

   // insert data
   var data = [
      { "a": "测试数据" },
      { "a": "abc" },
      { "a": "123" },
      { "a": " " },
      { "a": "爯爲𫌀𫖳䰲䰳" },
      { "a": "&*%$#@!" },
   ]
   cl.insert( data );
   var cursor = cl.find().hint( indexName );
   commCompareResults( cursor, data );

   // drop index
   commDropIndex( cl, indexName, true );
   assert.tryThrow( SDB_IXM_NOTEXIST, function()
   {
      cl.getIndex( indexName );
   } );
   commDropCS( db, csName );
}