/******************************************************************************
@Description :   seqDB-16016:  修改最大值  
@Modify list :   2018-10-22    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_16016";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   dbcl.insert({ a:1 });
   
   //alter attributes and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", MaxValue : 3, AcquireSize : 1, CacheSize : 2 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().MaxValue !== 3)
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
         cl.insert( { "a" : i, "b" : i } );
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
   var expRecs = [ { "a" : 1, "id1" : 1 }];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();