/******************************************************************************
@Description :   seqDB-16021:  修改当前值 
@Modify list :   2018-10-23    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16021";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   //insert records and check
   var coordNodes = getCoordNodeNames();
   var expRecs = [];
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : i, "b" : i } );
      expRecs.push({ "a" : i, "b" : i, "id1" : 1 + i*1000});
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs.sort(compare("id1")) );
   
   //alter attributes and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", CurrentValue : 500 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().CurrentValue !== 500)
   {
      throw "alter failed!";
   }
   
   //insert records and check
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : i, "b" : i } );
      expRecs.push({ "a" : i, "b" : i, "id1" : i*1000 + 501 });
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs.sort(compare("id1")) );
   
   //alter attributes and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", CurrentValue : 4000 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().CurrentValue !== 4000)
   {
      throw "alter failed!";
   }
   
   //insert records and check
   var coordNodes = getCoordNodeNames();
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      var rc = cl.find();
      while(rc.next())
      {}
      cl.insert( { "a" : i, "b" : i } );
      expRecs.push({ "a" : i, "b" : i, "id1" : i*1000 + 4001 });
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs.sort(compare("id1")) );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();