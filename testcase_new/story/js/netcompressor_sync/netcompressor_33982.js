/******************************************************************************
 * @Description   : seqDB-33982:开启消息压缩，写入大lob构造超长消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );
function test ()
{
   var csName = COMMCSNAME;
   var clName = "cl_33982";
   var lobNum = 10;
   var lobPathBase = WORKDIR + "netcompressor_large";
   try
   {
      File.remove( lobPathBase );
   }
   catch( e )
   {
      if( e != -4 )
      {
         throw e;
      }
   }
   var lobPath1 = lobPathBase + "/lob_33982_1";

   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );

      // 准备CL
      commDropCL( db, csName, clName, true, true, "drop CL in the beginning" );
      var cl = commCreateCL( db, csName, clName, { "ReplSize": -1 } );

      // 超大字符串
      var str = "";
      // aStr长度为1023
      var aStr = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
      for( var i = 0; i < 16 * 1024; i++ )
      {
         str = str + aStr;
      }
      // 创建目录
      File.mkdir( lobPathBase );
      // 写文件
      var lob = new File( lobPath1 );
      lob.write( str );
      lob.close();
      var expLobMd5 = File.md5( lobPath1 );

      // 重置数据库快照
      db.resetSnapshot( { "Type": "database" } );
      // 写入大lob
      for( i = 0; i < lobNum; i++ )
      {
         cl.putLob( lobPath1 );
      }
      // 检查lob
      var cursor = cl.listLobs();
      var actLobNum = 0;
      while( cursor.next() )
      {
         var lobId = cursor.current().toObj().Oid.$oid;
         var lobPath2 = lobPathBase + "/lob_33982_2_" + actLobNum;
         cl.getLob( lobId, lobPath2 );
         var actLobMd5 = File.md5( lobPath2 );
         assert.equal( actLobMd5, expLobMd5, "check lob md5" );
         actLobNum++;
      }
      assert.equal( actLobNum, lobNum, "check lob number" );

      commDropCL( db, csName, clName, true, false, "drop CL in the ending" );
      File.remove( lobPathBase );
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
