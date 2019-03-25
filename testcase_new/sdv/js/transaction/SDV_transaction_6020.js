/* *****************************************************************************
@discretion: 事务不存在执行提交
@author：2015-11-23 wuyan  Init
***************************************************************************** */

var clName = CHANGEDPREFIX + "_transaction6020";

function main()
{		
   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ;   
   }

   var cl = commCreateCL( db, COMMCSNAME, clName, 0, false, true, true ) ; 
   //commit transaction not exec beginTrans 
   try
   {
      execTransaction(commitTrans) ;
   }
   catch( e )
   {
      if ( e == "commitTrans() unknown error expect: -196" )
      {
         // think right
      }
      else
      {
         throw buildException("execTransaction(commitTrans)", e )
      }
   }      
   //@ clean end
	commDropCL( db, COMMCSNAME, clName, false, false,"drop CL in the beginning" );
}

main();

