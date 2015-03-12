/******************************************************************************
@Description : Test the transaction in sql.
@Modify list :
               2014-7-30  xiaojun Hu  Changed
******************************************************************************/
function main( db )
{
   // Judge transaction is enabled ?
   if ( commIsTransEnabled( db ) == false )
   {
      println( "transaction is not enabled" ) ;
      db.close() ;
      return ;
   }

   // Clear environment
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "drop cl in begin" ) ;
   // Create cl
   var varCL = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                             "create cl in begin" ) ;
   // Insert data
   sqlInsertAndCheck( db, COMMCSNAME, COMMCLNAME, 20, true, true,
                      "Insert 20 records" ) ;
   var count = varCL.count() ;
   println( "Query the " + varCL + "number : " + count ) ;
   println( "CsName : " + COMMCSNAME + "ClName : " + COMMCLNAME ) ;
   // Start transaction
   try
   {
      db.transBegin() ;
   }
   catch(e)
   {
      println("err happened when start transaction,rc="+e ) ;
      throw e ;
   }
   //database operations
   try
   {
     db.execUpdate( " update " + COMMCSNAME + "." + COMMCLNAME+
                    " set name=\"Tom\" where age=10" ) ;
   }
   catch(e)
   {
     println( "failed to update the record age=19,rc=" + e ) ;
     throw e ;
   }
   var rc ;
   try
   {
      rc = db.exec( "select * from " + COMMCSNAME + "." + COMMCLNAME +
                    " where name=\"Tom\"");
   }
   catch ( e )
   {
      println( "failed to read record name=Tom, error: " + e ) ;
      throw e ;
   }
   if ( 1 != rc.size() )
   {
      println( " get the record name=Tom happen err" ) ;
      throw -1;
   }

   try
   {
      db.close() ;
   }
   catch(e)
   {
      println( "Failed to disconnect,rc=" + e ) ;
      throw e ;
   }

   // Whether the update operation is successful after interruption
   var maxRollbackTime = 10 ;
   var timeCount = 0 ;
   var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
   var rc ;
   var size = 0 ;
   while ( true )
   {
      try
      {
         rc = db.exec( "select * from " + COMMCSNAME + "." + COMMCLNAME +
                       " where name=\"Tom\"");
         size = rc.size() ;
         println( "select data size: " + size ) ;
      }
      catch ( e )
      {
         println( "failed to read record name=Tom, rc= " + e ) ;
         throw e ;
      }

      if ( 0 != size )
      {
         if ( timeCount < maxRollbackTime )
         {
            sleep( 1000 ) ;
            timeCount++;
            continue ;
         }
         println( "Roll back failed or time out after connect close, size: " + size ) ;
         throw "Roll back failed or time out" ;
      }
      else
      {
         break ;
      }
   }

   var count = 0 ;
   // Clear environment
   while ( true )
   {
       ++count ;
	   try
	   {
		  db.execUpdate( "drop collection " + COMMCSNAME + "." + COMMCLNAME ) ;
		  break ;
	   }
	   catch (e)
	   {
	      if ( e == -190 && count <= 10 )
		  {
		     sleep( 1000 ) ;
		     // wait the rollback end for some seconds
		  }
		  else
		  {
		     println( "unexpected err happened when clear cl:" + e ) ;
		     throw e ;
          }
	   }
   }
}

main( db ) ;
