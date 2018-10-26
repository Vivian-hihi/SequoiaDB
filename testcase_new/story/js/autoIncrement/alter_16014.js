/******************************************************************************
@Description :   seqDB-16014:  修改起始值  
@Modify list :   2018-10-22    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }
    
   var clName = COMMCLNAME + "_16014";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   //alter attributes and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", StartValue : 1002 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().StartValue !== 1002)
   {
      throw "alter failed!";
   }
   
   //insert records and check
   var coordNodes = getCoordNodeNames();
   var expRecs = new Array();
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : 2, "b" : 2 } );
      expRecs.push({ "a" : 2, "b" : 2, "id1" : 1002 + i*1000});
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );   
}

main();