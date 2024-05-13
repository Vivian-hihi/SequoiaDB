/******************************************************************************
 * @Description   : seqDB-33947:开启消息压缩，构造重复率低的消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2024.05.10
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
testConf.skipStandAlone = true;
testPara.csName = COMMCSNAME + "_33980";
testPara.normalCLName = "cl_normal_33980";
testPara.shardCLName = "cl_shard_33980";

main( setUp, test );

function test (testpara)
{
   try
   {
      // 打开消息压缩
      db.updateConf( { "netcompressor": "lz4" } );

      var docs = insertLowDuplicateDocs( testpara.normalCL  );
      insertLowDuplicateDocs( testpara.shardCL );

      findAndcheckResult( testpara.normalCL, docs);
      findAndcheckResult( testpara.shardCL , docs);
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
