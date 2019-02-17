/* *****************************************************************************
@discretion: 重复开启事务,执行事务操作后提交
@author：2015-11-17 wuyan  Init
***************************************************************************** */

var clName = CHANGEDPREFIX + "_transaction002";

function main()
{		
	try
	{
      if( !commIsTransEnabled( db ) )
      {
         println( "transaction is disabled" ) ;   
      }

      var cl = commCreateCL( db, COMMCSNAME, clName, 0, false, true, true ) ; 
      var dataNum = 1000;
      var insert = new insertData( cl, dataNum ); 
      execTransaction(beginTrans,insert,beginTrans,commitTrans);
      checkResult( cl, true, insert ); 
        
	   //@ clean end
		commDropCL( db, COMMCSNAME, clName, false, false,"drop CL in the beginning" );
   }
   catch( e )
   {
      throw e;
   }
   finally
   {
      if ( undefined !== db )
      {
         db.close();
      }
   }
}
main();

