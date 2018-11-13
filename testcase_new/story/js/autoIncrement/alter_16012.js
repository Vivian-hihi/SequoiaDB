/******************************************************************************
@Description :   seqDB-16012:  修改自增字段名
@Modify list :   2018-10-22    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }
    
   var clName = COMMCLNAME + "_16012";
   var acquireSize = 10;
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1", AcquireSize : acquireSize } } );
   
   //insert records and check
   var coordNodes = getCoordNodeNames();
   var expRecs = [];
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : i } );
      expRecs.push({ "a" : i, "id1" : 1 + i*acquireSize});
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs.sort(compare("id1")) );
   
   try
   {
      dbcl.setAttributes({ AutoIncrement : { Field : "id2" } });
      throw "alter error!";
   }catch(e)
   {
      if(e !== -333)
      {
         throw e;
      }
   }
   
   //insert records and check
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : i } );
      expRecs.push({ "a" : i, "id1" : 2 + i*acquireSize});
      coord.close();
   }
    
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs.sort(compare("id1")) );
   
   commDropCL( db, COMMCSNAME, clName );
}

main();