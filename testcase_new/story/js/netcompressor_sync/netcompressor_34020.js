/******************************************************************************
 * @Description   : seqDB-34020:关闭消息压缩，读写数据
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );
function test ()
{
   var dmName = "dm_34020";
   var csName = COMMCSNAME + "_34020";
   var normalCLName = "cl_normal_34020";
   var shardCLName = "cl_shard_34020";
   var groupNames = commGetDataGroupNames( db );
   // 关闭消息压缩
   db.updateConf( { "netcompressor": "" } );

   // 准备CL
   commDropCS( db, csName, true );
   commDropDomain( db, dmName, true );
   db.createDomain( dmName, groupNames, { "AutoSplit": true } );
   var cs = db.createCS( csName, { "Domain": dmName } );
   var normalCL = cs.createCL( normalCLName, { "ReplSize": -1 } );
   var shardCL = cs.createCL( shardCLName, { "ShardingKey": { "a": 1 }, "AutoSplit": true, "ReplSize": -1 } );

   // 重置数据库快照
   db.resetSnapshot( { "Type": "database" } );

   // 读写数据
   for( var i = 0; i < 1000; i++ )
   {
      normalCL.insert( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i } );
      shardCL.insert( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i } );
   }
   findAndcheckResult( normalCL );
   findAndcheckResult( shardCL );

   // 查询数据库快照消息压缩相关指标
   var cursor = db.snapshot( SDB_SNAP_DATABASE, {}, { shardNetOut: "", shardUncompressed: "", shardCompressed: "", shardUncompressedCount: "", shardCompressedCount: "", replNetOut: "", replUncompressed: "", replCompressed: "", replUncompressedCount: "", replCompressedCount: "" } );
   var ncInfo = cursor.current().toObj();
   assert.notEqual( ncInfo.shardUncompressed, 0, "check shardUncompressed" );
   assert.equal( ncInfo.shardCompressed, 0, "check shardCompressed" );
   assert.notEqual( ncInfo.shardUncompressedCount, 0, "check shardUncompressedCount" );
   assert.equal( ncInfo.shardCompressedCount, 0, "check shardCompressedCount" );
   assert.notEqual( ncInfo.replUncompressed, 0, "check replUncompressed" );
   assert.equal( ncInfo.replCompressed, 0, "check replCompressed" );
   assert.notEqual( ncInfo.replUncompressedCount, 0, "check replUncompressedCount" );
   assert.equal( ncInfo.replCompressedCount, 0, "check replCompressedCount" );

   commDropCS( db, csName, false );
}

function findAndcheckResult ( cl )
{
   assert.equal( cl.count(), 1000 );
   for( var i = 0; i < 1000; i++ )
   {
      var cursor = cl.find( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i } );
      assert.equal( cursor.size(), 1 );
      cursor.close();
   }
}