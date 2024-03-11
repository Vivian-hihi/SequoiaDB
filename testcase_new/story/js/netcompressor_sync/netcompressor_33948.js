/******************************************************************************
 * @Description   : seqDB-33948:部分节点开启消息压缩，如1主1备，构造重复率高及低的消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );
function test ()
{
   var dmName = "dm_33948";
   var csName = COMMCSNAME + "_33948";
   var shardCLName = "cl_shard_33948";
   var groupNames = commGetDataGroupNames( db );
   var docNum = 1000;
   // 初始化：关闭消息压缩
   db.updateConf( { "netcompressor": "" } );
   try
   {
      // 准备CL
      commDropCS( db, csName, true );
      commDropDomain( db, dmName, true );
      db.createDomain( dmName, groupNames, { "AutoSplit": true } );
      var cs = db.createCS( csName, { "Domain": dmName } );
      var shardCL = cs.createCL( shardCLName, { "ShardingKey": { "a": 1 }, "AutoSplit": true, "ReplSize": -1 } );

      // 数据组1主1备开启消息压缩
      for( var i = 0; i < groupNames.length; i++ )
      {
         var rg = db.getRG( groupNames[i] );
         // 主节点
         var nodeName = rg.getMaster().toString();
         db.updateConf( { "netcompressor": "lz4" }, { "NodeName": nodeName } );
         // 备节点
         var nodeName = rg.getSlave().toString();
         db.updateConf( { "netcompressor": "lz4" }, { "NodeName": nodeName } );
      }
      // 编目组1主1备开启消息压缩
      var rg = db.getCataRG();
      // 主节点
      var nodeName = rg.getMaster().toString();
      db.updateConf( { "netcompressor": "lz4" }, { "NodeName": nodeName } );
      // 备节点
      var nodeName = rg.getSlave().toString();
      db.updateConf( { "netcompressor": "lz4" }, { "NodeName": nodeName } );

      // 重置数据库快照
      db.resetSnapshot( { "Type": "database" } );
      // 写重复率高的数据
      var expDocs = [];
      for( var i = 0; i < docNum; i++ )
      {
         var doc = { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i, "b": i };
         shardCL.insert( doc );
         expDocs.push( doc );
      }
      // 查询数据库快照消息压缩相关指标
      var cursor = db.snapshot( SDB_SNAP_DATABASE, {}, { shardNetOut: "", shardUncompressed: "", shardCompressed: "", shardUncompressedCount: "", shardCompressedCount: "", replNetOut: "", replUncompressed: "", replCompressed: "", replUncompressedCount: "", replCompressedCount: "" } );
      var ncInfo = cursor.current().toObj();
      assert.notEqual( ncInfo.shardUncompressed, 0, "check shardUncompressed" );
      assert.notEqual( ncInfo.shardCompressed, 0, "check shardCompressed" );
      assert.notEqual( ncInfo.shardUncompressedCount, 0, "check shardUncompressedCount" );
      assert.notEqual( ncInfo.shardCompressedCount, 0, "check shardCompressedCount" );
      assert.notEqual( ncInfo.replUncompressed, 0, "check replUncompressed" );
      assert.notEqual( ncInfo.replCompressed, 0, "check replCompressed" );
      assert.notEqual( ncInfo.replUncompressedCount, 0, "check replUncompressedCount" );
      assert.notEqual( ncInfo.replCompressedCount, 0, "check replCompressedCount" );
      // 检查写入结果
      assert.equal( shardCL.count(), docNum )
      commCompareResults( shardCL.find().sort( { "b": 1 } ), expDocs );
      shardCL.truncate();

      // 重置数据库快照
      db.resetSnapshot( { "Type": "database" } );
      // 写重复率低的数据
      var expDocs = [];
      for( var i = 0; i < docNum; i++ )
      {
         var doc = { "a": "abcdefghijklmnopqrstuvwxyz1234567890_" + i, "b": i };
         shardCL.insert( doc );
         expDocs.push( doc );
      }
      // 查询数据库快照消息压缩相关指标
      var cursor = db.snapshot( SDB_SNAP_DATABASE, {}, { shardNetOut: "", shardUncompressed: "", shardCompressed: "", shardUncompressedCount: "", shardCompressedCount: "", replNetOut: "", replUncompressed: "", replCompressed: "", replUncompressedCount: "", replCompressedCount: "" } );
      var ncInfo = cursor.current().toObj();
      assert.notEqual( ncInfo.shardUncompressed, 0, "check shardUncompressed" );
      assert.notEqual( ncInfo.shardCompressed, 0, "check shardCompressed" );
      assert.notEqual( ncInfo.shardUncompressedCount, 0, "check shardUncompressedCount" );
      assert.notEqual( ncInfo.shardCompressedCount, 0, "check shardCompressedCount" );
      assert.notEqual( ncInfo.replUncompressed, 0, "check replUncompressed" );
      assert.equal( ncInfo.replCompressed, 0, "check replCompressed" );
      assert.notEqual( ncInfo.replUncompressedCount, 0, "check replUncompressedCount" );
      assert.equal( ncInfo.replCompressedCount, 0, "check replCompressedCount" );
      // 检查写入结果
      commCompareResults( shardCL.find().sort( { "b": 1 } ), expDocs );

      commDropCS( db, csName, false );
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}