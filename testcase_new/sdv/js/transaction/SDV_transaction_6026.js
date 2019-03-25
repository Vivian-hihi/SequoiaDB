/************************************************************************
*@Description:	seqDB-6026:未开启事务，执行回滚_SD.transaction.037
*@Author:  		TingYU  2015/11/24
               wuyan 2017/1/6(执行回滚不报错)
************************************************************************/
main();

function main()
{
   var csName = COMMCSNAME + "_yt6026";
   var clName = COMMCLNAME + "_yt6026";
   
   try
   {
      if( !commIsTransEnabled( db ) )
      {
         println( "transaction is disabled" );
         return;
      }
      var cl = readyCL( csName, clName, {ReplSize:0} );
      
      //insert
      var dataNum = 100; 
      var insert = new insertData( cl, dataNum );
      execTransaction( insert );
      checkResult( cl, true, insert );
      
      //rollback
      try
      {   
         execTransaction( rollbackTrans );
         //throw buildException( "rollbackTrans()", "", "excute rollback after commit",
                              // -196, "did not throw any error" );
      }
      catch(e)
      {
         //var expErr = "rollbackTrans() unknown error expect: " + -196;
         //if( e !== expErr )
         //{
            throw e;
         //}
      }
      checkResult( cl, true, insert );
                    
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
