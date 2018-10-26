/******************************************************************************
@Description :   seqDB-15981:  集合中不存在记录，创建/删除自增字段 
@Modify list :   2018-10-15    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_15981";
   var field = "id1";
   var cacheSize = 10;
   var acquireSize = 1;
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   
   dbcl.createAutoIncrement( { Field : field, CacheSize : 10, AcquireSize : 1 } );
   
   //check sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceName = "SYS_" + clID + "_" + field + "_SEQ";
   var expSequenceObj = { CacheSize : cacheSize, AcquireSize : acquireSize };
   checkSequence( sequenceName, expSequenceObj  );

   dbcl.insert( { id1 : 5, a : 7 } );

   var rc = dbcl.find();
   var expRecs = [ { id1 : 5, a : 7 } ];
   checkRec( rc, expRecs ); 

   dbcl.update( { $set :{ a : 77 } }, { id1 : 5 } );

   rc = dbcl.find();
   expRecs = [ { id1 : 5, a : 77 } ];
   checkRec( rc, expRecs );

   dbcl.dropAutoIncrement( field );

   var cursor = db.snapshot( 8, { Name : COMMCSNAME + "." + clName } );
   if( cursor.current().toObj().AutoIncrement.length !== 0 )
   {
      throw "drop autoIncrement failed!";
   }       
   
   commDropCL( db, COMMCSNAME, clName );
}



main();