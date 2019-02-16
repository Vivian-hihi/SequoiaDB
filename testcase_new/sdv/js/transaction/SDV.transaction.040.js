/************************************************************************
*@Description:	seqDB-6029:重复执行事务回滚_SD.transaction.040
               依次执行开启事务、删除、回滚、回滚操作
*@Author:  		TingYU  2015/11/24
               wuyan 2017/1/6(修改重复执行回滚不报错)
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
         return;
      }
      var cl = readyCL( csName, clName, {ReplSize:0} );
      
      //begin and rollback
      var dataNum = 100; 
      var insert = new insertData( cl, dataNum );
      var remove = new removeData ( cl ); 
      execTransaction( insert, beginTrans, remove, rollbackTrans );
      checkResult( cl, false, remove );
      
      //rollback again
      try
      {   
         execTransaction( rollbackTrans );
         //throw buildException( "rollbackTrans()", "", "excute rollback after commit",
                               //-196, "did not throw any error" );
      }
      catch(e)
      {
         //var expErr = "rollbackTrans() unknown error expect: " + -196;
         //if( e !== expErr )
        // {
            throw e;
         //}
      }
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
