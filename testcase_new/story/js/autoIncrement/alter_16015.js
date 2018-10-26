/******************************************************************************
@Description :   seqDB-16015:  修改最小值   
@Modify list :   2018-10-22    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
   
   var clName = COMMCLNAME + "_16015";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
 
   //alter attribute and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", Increment : -1, StartValue : 3, MinValue : 2, 
                        MaxValue : 10, AcquireSize : 1, CacheSize : 5 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().MinValue !== 2)
   {
      throw "alter failed!";
   }
   
   //insert records and check
   var coordNodes = getCoordNodeNames();
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      try
      {
         for(var j = 0; j < 2; j++ )
         {
            cl.insert( { "a" : i, "b" : i } );
         }
      }catch(e)
      {
         if(e !== -325)
         {
            throw e;
         }
      }
      coord.close();      
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   var expRecs = [{ "a" : 0, "b" : 0, "id1" : 2 },
                  { "a" : 0, "b" : 0, "id1" : 3 }];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();