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
   var acquireSize = 5;
   var maxValue = 40;
   var cacheSize = 10;
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1", AcquireSize : 10 } } );
   
   //insert records and check
   var coordNodes = getCoordNodeNames();
   if(coordNodes.length !== 3)
   {
      return ;
   }
   var expRecs = [];
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : i, "b" : i } );
      expRecs.push({ "a" : i, "b" : i, "id1" : 1 + i*10});
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs );
   
   //alter attributes default and check
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", MaxValue : maxValue, AcquireSize : acquireSize, CacheSize : cacheSize } });
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id1_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().Cycled !== false)
   {
      throw "alter failed!";
   }
 
   //insert records and check
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      try
      {
         cl.insert( { "a" : i, "b" : i } );
         expRecs.push({ "a" : i, "b" : i, "id1" : 1 + coordNodes.length*10 + i*acquireSize });
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
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      var rc = cl.find();
      while(rc.next())
      {}
      cl.insert( { "a" : i, "b" : i } );
      expRecs.push({ "a" : i, "b" : i, "id1" : 1 + i*acquireSize });
      coord.close();
   }
   
   var rc = dbcl.find();
   checkRec( rc, expRecs );
   println("===check Cycled true succeed!===");
 
   //alter Cycled false
   dbcl.setAttributes({ AutoIncrement : { Field : "id1", Cycled : false, AcquireSize : 3, CacheSize : 6, MaxValue : 20} });
   
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().Cycled !== false )
   {
      throw "alter failed!";
   }
   
   //insert records and check
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      var rc = cl.find();
      while(rc.next())
      {}
      try
      {
         cl.insert( { "a" : i, "b" : i } );
         expRecs.push({ "a" : i, "b" : i, "id1" : 1 + coordNodes.length*acquireSize + i*3 });
      }catch(e)
      {
         if(e !== -325)
         {
            throw "insert error!";
         }
      }
      coord.close();
   }
      
   var rc = dbcl.find().sort({ "id1" : 1 });
   checkRec( rc, expRecs.sort(compare("id1")) );
   println("===check Cycled false succeed!===");   

   commDropCL( db, COMMCSNAME, clName );
}
main();
