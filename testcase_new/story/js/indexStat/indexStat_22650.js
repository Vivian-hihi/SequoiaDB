/***************************************************************************
@Description : seqDB-22650: 数据节点未生成SYSTAT.SYSINDEXSTAT时，获取索引统计信息 
@Modify list : Zhao Xiaoni 2020/8/19
****************************************************************************/
testConf.skipStandAlone = true;
testConf.clName = "cl_22650";

main( test );

function test( testPara )
{
   commCreateIndex ( testPara.testCL, "index_22650", { "a": 1 } );
   var records = [];
   for( var i = 0; i < 200; i++ )
   {
      records.push( { "a": i } );
   }
   testPara.testCL.insert( records );

   try
   {
      testPara.testCL.getIndexStat( "index_22650" ).toObj();
      throw "It should be error: -349";
   }
   catch( e )
   {
      if( e != -349 )
      {
         throw new Error( e );
      }
   }
}


