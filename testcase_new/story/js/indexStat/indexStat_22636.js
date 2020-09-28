/***************************************************************************
@Description : seqDB-22636: 获取集合索引统计信息接口参数验证 
@Modify list : Zhao Xiaoni 2020/8/19
****************************************************************************/
testConf.clName = "cl_22636";

main( test );

function test( testPara )
{
   assert.tryThrow( -259, function()
   {
      testPara.testCL.getIndexStat();
   });
   assert.tryThrow( -349, function()
   {  
      testPara.testCL.getIndexStat( "indexName" );
   });
   assert.tryThrow( -6, function()
   {  
      testPara.testCL.getIndexStat( { "indexName": 1 } );
   });
}


