/************************************
*@Description: seqDB-33941:设置字符集为GB18030，查看getLastErrObj()信息
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/

main( test );
function test ()
{
   var csName = "集合空间_33941";
   var clName = "集合_33941";
   var indexName = "索引_33941";
   db.setCharsets( "GB18030" );

   commDropCS( db, csName );
   var cl = commCreateCL( db, csName, clName );

   // create unique index
   commCreateIndex( cl, indexName, { age: 1 }, { unique: true } )
   //insert duplicate data
   assert.tryThrow( SDB_IXM_DUP_KEY, function()
   {
      cl.insert( [{ age: 1, name: "zhangsan" }, { age: 1, name: "lisi" }] )
   } );
   var errObj = getLastErrObj().toObj();
   assert.equal( errObj.ErrNodes[0].ErrInfo.IndexName, indexName, "indexName display not expected" );
   assert.equal( errObj.IndexName, indexName, "indexName display not expected" );

   commDropCS( db, csName );
}