/******************************************************************************
@Description :   seqDB-17068:开启事务前执行事务提交和回滚
@Modify list :   2019-1-9    xiaoni Zhao  Init
******************************************************************************/
function main( db )
{
   try
   {
      db.transCommit() ;
      throw "transCommit error!";
   }
   catch( e )
   {
      if( -196 != e )
      {
         throw e ;
      }
   }
   
   try
   {
      db.transRollback() ;
      throw "transRollback error!";
   }
   catch( e )
   {
      if( -196 != e )
      {
         throw e ;
      }
   }
}
main();
