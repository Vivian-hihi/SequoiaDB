/***************************************************************************
@Description : seqDB-22636: 获取集合索引统计信息接口参数验证 
@Modify list : Zhao Xiaoni 2020/8/19
****************************************************************************/
testConf.clName = "cl_22636";

main( test );

function test( testPara )
{
   try
   {
      testPara.testCL.getIndexStat();
      throw "It should be error: -259";
   }
   catch( e ) 
   {
      if( e !== -259 )
      {
         throw new Error( e );
      }
   }

   try
   {
      testPara.testCL.getIndexStat( "indexName" );
      throw "It should be error: -349";
   }
   catch( e )
   {
      if( e !== -349 )
      {
         throw new Error( e );
      }
   }   

   try
   {
      testPara.testCL.getIndexStat( { "indexName": 1 } );
      throw "It should be error: -6";
   }
   catch( e )
   {
      if( e !== -6 )
      {
         throw new Error( e );
      }
   }
}


