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
   
   dbcl.insert({ a:1 });
   
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
   var coordNodes = getCoordNodeNames();
   var expRecs = [];
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      try
      { 
         cl.insert( { "a" : i, "b" : i } );
         expRecs.push({ "a" : i, "b" : i,  });
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
   var expRecs = [ { "a" : 1, "id1" : 1 },
                   { "a" : 1, "id1" : 501 },
                   ];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();