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
   checkRec( rc, expRecs );
   
   //alter attributes and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", MaxValue : 5000 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   var currentValue = cursor.current().toObj().CurrentValue;
   if( cursor.current().toObj().MaxValue !== 5000)
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
         expRecs.push({ "a" : i, "b" : i, "id1" : currentValue + i*1000 });
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
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();