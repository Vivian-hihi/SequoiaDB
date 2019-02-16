/************************************************************************
*@Description:	执行更新+提交事务操作，执行删除+回滚事务操作
               seqDB-6021:执行事务回滚_SD.transaction.032
*@Author:  		TingYU  2015/11/23
************************************************************************/
// SEQUOIADBMAINSTREAM-4182
// main();

function main()
{
   var csName = COMMCSNAME + "_yt";
   var clName = COMMCLNAME + "_yt";
   
   try
   {
      if( !commIsTransEnabled( db ) )
      {
         println( "transaction is disabled" );
         return; //告诉吴艳 加return
      }
      var cl = readyCL( csName, clName, {ReplSize:0} );
      
      var dataNum = 100; 
      var insert = new insertData( cl, dataNum );
      var update = new updateData ( cl ); 
      execTransaction( insert, beginTrans, update, commitTrans );
      checkResult( cl, true, update );
      
      var remove = new removeData( cl );
      execTransaction( beginTrans, remove );
      checkResult( cl, true, remove );
      
      execTransaction( rollbackTrans );
      checkResult( cl, false, remove );  
                    
	   clean( csName, clName );
   }
   catch( e )
   {
      throw e;
   }
   finally
   {
   }
}
