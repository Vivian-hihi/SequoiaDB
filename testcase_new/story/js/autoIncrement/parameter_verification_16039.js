/******************************************************************************
@Description :   seqDB-16039:  CacheSize字段参数校验 
@Modify list :   2018-10-23    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16039";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "a0"} } );
   
   dbcl.createAutoIncrement([{ Field : "a1", CacheSize : 2147483647 },
                             { Field : "a2", CacheSize : 2000 }]);
   
   createAutoIncrement(dbcl, "a3", -10000);
   
   createAutoIncrement(dbcl, "a4", 2147483648);
   
   createAutoIncrement(dbcl, "a5", 0);
   
   createAutoIncrement(dbcl, "a6", 123.4);
   
   createAutoIncrement(dbcl, "a7", { $decimal:"123.456" });
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a0_SEQ",
                        "SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a2_SEQ"]; 
   var expSequences =  [{},
                        { CacheSize : 2147483647 },
                        { CacheSize : 2000 }];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   //insert records
   var coordNodes = getCoordNodeNames();
   var expRecs = [];
   for( var i = 0; i < coordNodes.length; i++ )
   {
      var coord = new Sdb( coordNodes[ i ] );
      var cl = coord.getCS( COMMCSNAME ).getCL( clName );
      cl.insert( { "a" : i } );
      expRecs.push({ "a" : i, "a0" : 1 + i*1000, "a1" : 1 + i*1000, "a2" : 1 + i*1000 });
      coord.close();
   }
  
   var rc = dbcl.find();
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement(dbcl, field, cacheSize)
{
   try
   {
      dbcl.createAutoIncrement({ Field : field, CacheSize : cacheSize });
      throw "create error!";
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
}

main();