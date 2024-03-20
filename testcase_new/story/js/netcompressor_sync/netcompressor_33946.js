/******************************************************************************
 * @Description   : seqDB-33946:开启消息压缩，构造重复率高的消息
 *                  seqDB-33984:数据库快照新增消息压缩字段正确性检查
 *                  seqDB-33985:resetSnapshot重置快照，检查消息压缩相关字段正确性
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );
function test ()
{
   var dmName = "dm_33946";
   var csName = COMMCSNAME + "_33946";
   var normalCLName = "cl_normal_33946";
   var shardCLName = "cl_shard_33946";
   var groupNames = commGetDataGroupNames( db );
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );

      // 准备CL
      commDropCS( db, csName, true );
      commDropDomain( db, dmName, true );
      db.createDomain( dmName, groupNames, { "AutoSplit": true } );
      var cs = db.createCS( csName, { "Domain": dmName } );
      var normalCL = cs.createCL( normalCLName, { "ReplSize": -1 } );
      var shardCL = cs.createCL( shardCLName, { "ShardingKey": { "a": 1 }, "AutoSplit": true, "ReplSize": -1 } );

      // 重置数据库快照 -> 普通表写数据 -> 查询数据库快照消息压缩相关指标
      db.resetSnapshot( { "Type": "database" } );
      insertDocs( normalCL );
      snapshotAndCheckResult( "normal cl insert docs" );

      // 重置数据库快照 -> 分区表写数据 -> 查询数据库快照消息压缩相关指标
      db.resetSnapshot( { "Type": "database" } );
      insertDocs( shardCL );
      snapshotAndCheckResult( "shard cl insert docs" );

      // 重置数据库快照 -> 普通表和分区表读数据 -> 查询数据库快照消息压缩相关指标
      db.resetSnapshot( { "Type": "database" } );
      findAndcheckResult( normalCL );
      findAndcheckResult( shardCL );
      // 读数据不涉及shardCompressedCount和replCompressed
      var cursor = db.snapshot( SDB_SNAP_DATABASE, {}, { shardNetOut: "", shardUncompressed: "", shardCompressed: "", shardUncompressedCount: "", shardCompressedCount: "", replNetOut: "", replUncompressed: "", replCompressed: "", replUncompressedCount: "", replCompressedCount: "" } );
      var ncInfo = cursor.current().toObj();
      var message = "find docs";
      assert.notEqual( ncInfo.shardCompressed, 0, message + ", check shardCompressed" );
      assert.notEqual( ncInfo.shardCompressedCount, 0, message + ", check shardCompressedCount" );
      assert.equal( ncInfo.replCompressed, 0, message + ", check replCompressed" );
      assert.equal( ncInfo.replCompressedCount, 0, message + ", check replCompressedCount" );

      commDropCS( db, csName, false );
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}

function insertDocs ( cl )
{
   for( var i = 0; i < 1000; i++ )
   {
      cl.insert( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i } );
   }
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

function snapshotAndCheckResult ( message )
{
   var cursor = db.snapshot( SDB_SNAP_DATABASE, {}, { shardNetOut: "", shardUncompressed: "", shardCompressed: "", shardUncompressedCount: "", shardCompressedCount: "", replNetOut: "", replUncompressed: "", replCompressed: "", replUncompressedCount: "", replCompressedCount: "" } );
   var ncInfo = cursor.current().toObj();
   assert.notEqual( ncInfo.shardUncompressed, 0, message + ", check shardUncompressed" );
   assert.notEqual( ncInfo.shardCompressed, 0, message + ", check shardCompressed" );
   assert.notEqual( ncInfo.shardCompressedCount, 0, message + ", check shardCompressedCount" );
   assert.notEqual( ncInfo.shardUncompressedCount, 0, message + ", check shardUncompressedCount" );
   assert.notEqual( ncInfo.replUncompressed, 0, message + ", check replUncompressed" );
   assert.notEqual( ncInfo.replCompressed, 0, message + ", check replCompressed" );
   assert.notEqual( ncInfo.replUncompressedCount, 0, message + ", check replUncompressedCount" );
   assert.notEqual( ncInfo.replCompressedCount, 0, message + ", check replCompressedCount" );
}

