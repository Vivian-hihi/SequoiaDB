/******************************************************************************
 * @Description   : seqDB-34020:关闭消息压缩，读写数据
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;
testPara.csName = COMMCSNAME + "_34020";
testPara.normalCLName = "cl_normal_34020";
testPara.shardCLName = "cl_shard_34020";

main( setUp, test );

function test (testpara)
{
   // 关闭消息压缩
   db.updateConf( { "netcompressor": "" } );
   var docs = insertHighDuplicateDocs( testpara.normalCL );
   // 分区表写数据
   insertHighDuplicateDocs( testpara.shardCL );

   findAndcheckResult( testpara.normalCL, docs);
   findAndcheckResult( testpara.shardCL, docs);
}
