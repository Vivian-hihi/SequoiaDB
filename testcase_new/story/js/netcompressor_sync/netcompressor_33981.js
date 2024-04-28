/******************************************************************************
 * @Description   : seqDB-33981:开启消息压缩，单条/批量写入超长bson构造超长消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );
function test ()
{
   var csName = COMMCSNAME + "_33981";
   var clName = "cl_33981";
   var docNum = 10;
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );

      // 超大字符串
      var str = "";
      // aStr长度为1023
      var aStr = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
      for( var i = 0; i < 16 * 1024; i++ )
      {
         str = str + aStr;
      }

      // 准备CL
      commDropCS( db, csName, true );
      var cs = db.createCS( csName );
      var cl = cs.createCL( clName, { "ReplSize": -1 } );

      // 重置数据库快照
      db.resetSnapshot( { "Type": "database" } );
      // 单条写入
      var expDocs = [];
      for( i = 0; i < docNum; i++ )
      {
         var doc = { "a": str + "_" + i, "b": i };
         cl.insert( doc );
         expDocs.push( doc );
      };
      // 检查结果
      commCompareResults( cl.find().sort( { "b": 1 } ), expDocs );
      cl.remove();

      // 重置数据库快照
      db.resetSnapshot( { "Type": "database" } );
      // 批量写入
      var docs = [];
      for( i = 0; i < docNum; i++ )
      {
         var doc = { "a": str + "_" + i, "b": i };
         docs.push( doc );
      };
      cl.insert( docs );
      commCompareResults( cl.find().sort( { "b": 1 } ), docs );

      commDropCS( db, csName, false );
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}

