/************************************************************************
*@Description:	seqDB-6025:事务提交后执行回滚_SD.transaction.036
               依次执行开启事务、更新、提交、回滚
*@Author:  		TingYU  2015/11/24
               wuyan 2017/1/6(修改重复执行回滚不报错) 
************************************************************************/
main();
function main()
{
   var csName = COMMCSNAME + "_yt6025";
   var clName = COMMCLNAME + "_yt6025";
   
   try
   {
      if( !commIsTransEnabled( db ) )
      {
         println( "transaction is disabled" );
         return;
      }
      var cl = readyCL( csName, clName, {ReplSize:0} );
      
      //begin and commit
      var dataNum = 100; 
      var insert = new insertData( cl, dataNum );
      var update = new updateData ( cl ); 
      execTransaction( insert, beginTrans, update, commitTrans );
      checkResult( cl, true, update );
      
      //rollback
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
         //{
            throw e;
         //}
      }
      checkResult( cl, true, update );
                    
	   clean( csName, clName );
   }
   catch( e )
   {
      throw e;
   }   
}
