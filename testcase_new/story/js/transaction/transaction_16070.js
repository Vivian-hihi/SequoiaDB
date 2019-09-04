/* *****************************************************************************
@discretion: there is a transaction operation on the cl ,than rename cl
@author£º2018-10-16 wuyan  Init
***************************************************************************** */
main(db);
function main(db)
{		
	try
	{
      if( !commIsTransEnabled( db ) )
      {
         println( "transaction is disabled" ) ; 
         return;  
      }
      var clName = CHANGEDPREFIX + "_renameCL16070";
      var newCLName = CHANGEDPREFIX + "_newrenameCL16070";
      commDropCL( db, COMMCSNAME, clName, true, true, "clear collection in the beginning" ) ; 
      commDropCL( db, COMMCSNAME, newCLName, true, true, "clear collection in the beginning" ) ; 
      var dbcl = commCreateCL( db, COMMCSNAME, clName, 0, false, true, true ) ;          
      
      var dataNums = 100;
      beginTrans( db );
      //var dbcl = db.getCS( COMMCSNAME ).getCL( clName ); 
      insertData( dbcl, dataNums );
      
      var newdb = new Sdb(COORDHOSTNAME, COORDSVCNAME ) ;
      renameCLExistTrans( newdb, COMMCSNAME, clName, newCLName );
      //check the clName is oldName
      checkRenameCLResult( COMMCSNAME, newCLName, clName);
      
      commitTrans( db );
      
      renameCLNoTrans( newdb, COMMCSNAME, clName, newCLName );
      checkRenameCLResult( COMMCSNAME, clName, newCLName); 
      checkDatas( COMMCSNAME, newCLName, dataNums );     

		commDropCL( db, COMMCSNAME, newCLName, true, true,"drop CL in the ending" );
   }
   finally
   {
      if ( undefined !== newdb )
      {
         newdb.close();
      }
   }
}

function renameCLExistTrans( db, csName, clName, newCLName )
{
   try
   {
      println( "---Begin to rename cl, the cl exist transaction" ) ;
      var dbcs = db.getCS( csName );
      dbcs.renameCL( clName, newCLName );
   }
   catch( e )
   {
      if ( -190 != e )
      {
         throw new Error(e);
      }      
   }
}

function renameCLNoTrans( db, csName, clName, newCLName )
{
   println( "---Begin to rename cl, the cl no transaction" ) ;
   var dbcs = db.getCS( csName );
   dbcs.renameCL( clName, newCLName );
}

function checkDatas( csName, newCLName, expRecordNums )
{   
   println("---Begin to check the records");
   var dbcl = db.getCS( csName ).getCL( newCLName );
   var count = dbcl.count();      
   if( count != expRecordNums  )
   {
      throw new Error("expect record num: " + expRecordNums + "actual record num: " + count);
   }  
}

