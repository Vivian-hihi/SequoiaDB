/******************************************************************************
@Description :   seqDB-16019:  修改Cycled属性值   
@Modify list :   2018-10-22    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16019";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1", } } );
   
   dbcl.insert({ a:1 });
   
   //alter attributes default and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", MaxValue : 3, AcquireSize : 1, CacheSize : 1 } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().Cycled !== false)
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
    
   var rc = dbcl.find();
   var expRecs = [ { "a" : 1, "id1" : 1 }];
   checkRec( rc, expRecs );
   println("===check Cycled default succeed!===");
   
   //alter Cycled true
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", Cycled : true } });
   
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().Cycled !== true)
   {
      throw "alter failed!";
   }
   
   //insert records and check
   var expRecs = [{ "a" : 1, "id1" : 1 }];
   for( var i = 0; i < coordNodes.length; i++ )
   {
      if(coordNodes[i] !== System.getHostName() + "11810" )
      {
         var coord = new Sdb(coordNodes[i]);
         var cl = coord.getCS(COMMCSNAME).getCL(clName);
         var rc = cl.find();
         while(rc.next())
         {}
         cl.insert( { "a" : i, "b" : i } );
         expRecs.push({ "a" : i, "b" : i, "id1" : 1 });
      }
      coord.close();
      break;
   }
   
   var rc = dbcl.find();
   checkRec( rc, expRecs );
   println("===check Cycled true succeed!===");
 
   //alter Cycled false
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", Cycled : false } });
   
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().Cycled !== false )
   {
      throw "alter failed!";
   }
   
   //insert records and check
   try
   { 
      for(var j = 0; j < 3; j++)
      {
         dbcl.insert( { "a" : j, "b" : j } );
         expRecs.push({ "a" : j, "b" : j, "id1" : 2 + j });
      }  
   }catch(e)
   {
      if(e !== -325)
      {
         throw e;
      }
   }
      
   var rc = dbcl.find().sort({ "id1" : 1 });
   checkRec( rc, expRecs );
   println("===check Cycled false succeed!===");   

   commDropCL( db, COMMCSNAME, clName );
}
main();
