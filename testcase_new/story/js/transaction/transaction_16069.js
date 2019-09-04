/* *****************************************************************************
@discretion: there is a transaction operation on the cl ,than rename cl in transaction
@author£º2018-10-16 wuyan  Init
***************************************************************************** */
main(db);
function main(db)
{		

   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ; 
      return;  
   }
   var clName = CHANGEDPREFIX + "_renameCL16069";
   var newCLName = CHANGEDPREFIX + "_newrenameCL16069";
   commDropCL( db, COMMCSNAME, clName, true, true, "clear collection in the beginning" ) ; 
   commDropCL( db, COMMCSNAME, newCLName, true, true, "clear collection in the beginning" ) ; 
   var dbcl = commCreateCL( db, COMMCSNAME, clName, 0, false, true, true ) ;          
      
   var dataNums = 100;
   beginTrans( db );      
   insertData( dbcl, dataNums );      
      
   //rename cl fail in a transction, check the clName is oldName
   renameCLInTrans( db, COMMCSNAME, clName, newCLName );      
   checkRenameCLResult( COMMCSNAME, newCLName, clName);
      
   commitTrans( db );
   checkDatas( COMMCSNAME, clName, dataNums ); 
      
   //rename cl success after commit the transction
   renameCLNoTrans( db, COMMCSNAME, clName, newCLName );
   checkRenameCLResult( COMMCSNAME, clName, newCLName); 
   checkDatas( COMMCSNAME, newCLName, dataNums );     

   commDropCL( db, COMMCSNAME, newCLName, true, true,"drop CL in the ending" );    
}

function renameCLInTrans( db, csName, clName, newCLName )
{
   try
   {
      println( "---Begin to rename cl in a transaction" ) ;
      var dbcs = db.getCS( csName );
      dbcs.renameCL( clName, newCLName );
   }
   catch( e )
   {
      if ( -336 !== e )
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

