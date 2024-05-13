/******************************************************************************
 * @Description   : seqDB-33981:开启消息压缩，单条/批量写入超长bson构造超长消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2024.05.11
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
testConf.skipStandAlone = true;
testConf.csName = COMMCSNAME + "_33981";
testConf.clName = "cl_33981";
testConf.clOpt = { "ReplSize": -1 }

main( test );
function insertBigHighDuplicateDocs( cl )
{
   var docNum = 10;
   // 超大字符串
   var str = "";
   var aStr = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
   for( var i = 0; i < 16 * 1024; i++ )
   {
       str = str + aStr;
   }

   // 单条写入
   var docs = [];
   for( i = 0; i < docNum; i++ )
   {
       var doc = {"a": str + "_" + i,b:i};
       cl.insert( doc );
       docs.push( doc );
   }
   return docs;
}

function test (testpara)
{
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );
      // 单条插入
      var docs = insertBigHighDuplicateDocs(testpara.testCL) ;
      findAndcheckResult(testpara.testCL, docs);
    
      testpara.testCL.remove();

      // 批量写入
      testpara.testCL.insert( docs );
      findAndcheckResult(testpara.testCL, docs);
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
