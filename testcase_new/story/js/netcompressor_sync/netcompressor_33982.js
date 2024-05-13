/******************************************************************************
 * @Description   : seqDB-33982:开启消息压缩，写入大lob构造超长消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2024.05.11
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
testConf.skipStandAlone = true;
testConf.clName = "cl_33982";
testConf.clOpt = { "ReplSize": -1 }

main( setUp, test, tearDown );
function setUp(testpara)
{
   testpara.lobPathBase = WORKDIR + "netcompressor_large";
   try
   {
      File.remove( testpara.lobPathBase );
   }
   catch( e )
   {
      if( e != -4 )
      {
         throw e;
      }
   }
   
   // 创建目录
   File.mkdir( testpara.lobPathBase );
   testpara.lobPath1 = testpara.lobPathBase + "/lob_33982_1";
   var lob = new File( testpara.lobPath1 );
   
   // aStr长度为1023
   var aStr = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
   for( var i = 0; i < 16 * 1024; i++ )
   {
      lob.write( aStr );
   } 
   lob.close();
   testpara.lobMd5 = File.md5( testpara.lobPath1 );
}

function checkLobContentAndNumber(cl, lobMd5, number, basePath)
{
   // 检查lob
   var cursor = cl.listLobs();
   var actLobNum = 0;
   while( cursor.next() )
   {
       var lobId = cursor.current().toObj().Oid.$oid;
       var lobPath2 = basePath + "/lob_33982_2_" + actLobNum;
       cl.getLob( lobId, lobPath2 );
       var actLobMd5 = File.md5( lobPath2 );
       assert.equal( actLobMd5, lobMd5, "check lob md5" );
       actLobNum++;
   }
   assert.equal( actLobNum, number, "check lob number" );
}
function test (testpara)
{
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );
  
      var lobNum = 10;
      // 写入大lob
      for( i = 0; i < lobNum; i++ )
      {
         testpara.testCL.putLob( testpara.lobPath1 );
      }
    
    checkLobContentAndNumber(testpara.testCL, testpara.lobMd5, lobNum, testpara.lobPathBase);
      
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}

function tearDown(testpara)
{
   File.remove( testpara.lobPathBase );
}
