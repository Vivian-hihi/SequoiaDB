/* *****************************************************************************
@discretion: there is a transaction operation on the cl1 ,than rename cl2,the cl1 and cl2 on the same cs
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
   
   var csName = CHANGEDPREFIX + "_renamecs16071";
   commDropCS( db, csName, true, "drop CS in the beginning"); 
   
   var clName1 = CHANGEDPREFIX + "_renameCL16071a";
   var newCLName1 = CHANGEDPREFIX + "_newrenameCL16071a";
   var clName2 = CHANGEDPREFIX + "_renameCL16071b";           
   var dbcs = commCreateCS( db, csName, false, "Failed to create CS.");    
   commCreateCL( db, csName, clName1, 0, false, true, true ) ; 
   var dbcl = commCreateCL( db, csName, clName2, 0, false, true, true ) ;          
   
   var dataNums = 100;
   beginTrans( db );     
   insertData( dbcl, dataNums ); 
   
   renameCL( db, csName, clName1, newCLName1 );
         
   commitTrans( db );     
   
   checkRenameCLResult( csName, clName1, newCLName1 ); 
   checkDatas( csName, clName2, dataNums );     

	commDropCS( db, csName, true, "drop CS in the ending");  
}

function renameCL( db, csName, clName, newCLName )
{
   println( "---Begin to rename cl" ) ;
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

