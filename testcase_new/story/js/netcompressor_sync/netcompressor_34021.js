/******************************************************************************
 * @Description   : seqDB-34021:独立模式，开启消息压缩，构造重复率高的消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.05
 * @LastEditTime  : 2023.03.05
 * @LastEditors   : huangxiaoni
 ******************************************************************************/

main( test );
function test ()
{
   if( !commIsStandalone( db ) )
   {
      return;
   }

   var csName = COMMCSNAME + "_34021";
   var clName = "cl_normal_34021";
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );

      // 准备CL
      commDropCL( db, csName, clName, true, true, "drop cl in the beginning" );
      var cl = commCreateCL( db, csName, clName );

      // 重置数据库快照
      db.resetSnapshot( { "Type": "database" } );
      for( var i = 0; i < 1000; i++ )
      {
         cl.insert( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i } );
      }
      // 检查数据库快照消息压缩相关指标
      var cursor = db.snapshot( SDB_SNAP_DATABASE, {}, { shardNetOut: "", shardUncompressed: "", shardCompressed: "", shardUncompressedCount: "", shardCompressedCount: "", replNetOut: "", replUncompressed: "", replCompressed: "", replUncompressedCount: "", replCompressedCount: "" } );
      var ncInfo = cursor.current().toObj();
      assert.equal( ncInfo.shardUncompressed, 0, "check shardUncompressed" );
      assert.equal( ncInfo.shardCompressed, 0, "check shardCompressed" );
      assert.equal( ncInfo.shardCompressedCount, 0, "check shardCompressedCount" );
      assert.equal( ncInfo.shardUncompressedCount, 0, "check shardUncompressedCount" );
      assert.equal( ncInfo.replUncompressed, 0, "check replUncompressed" );
      assert.equal( ncInfo.replCompressed, 0, "check replCompressed" );
      assert.equal( ncInfo.replUncompressedCount, 0, "check replUncompressedCount" );
      assert.equal( ncInfo.replCompressedCount, 0, "check replCompressedCount" );
      // 检查数据正确性
      assert.equal( cl.count(), 1000 );
      for( var i = 0; i < 1000; i++ )
      {
         var cursor = cl.find( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i } );
         assert.equal( cursor.size(), 1 );
         cursor.close();
      }

      commDropCL( db, csName, clName, true, false, "drop cl in the endding" );
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
