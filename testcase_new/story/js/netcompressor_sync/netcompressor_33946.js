/******************************************************************************
 * @Description   : seqDB-33946:开启消息压缩，构造重复率高的消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2024.05.10
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
testConf.skipStandAlone = true;
testPara.csName = COMMCSNAME + "_33946";
testPara.normalCLName = "cl_normal_33946";
testPara.shardCLName = "cl_shard_33946";

main( setUp, test );
function test (testpara)
{
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );

      // 普通表写数据
      var docs = insertHighDuplicateDocs( testpara.normalCL );
      // 分区表写数据
      insertHighDuplicateDocs( testpara.shardCL );

      findAndcheckResult( testpara.normalCL, docs);
      findAndcheckResult( testpara.shardCL, docs);
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
