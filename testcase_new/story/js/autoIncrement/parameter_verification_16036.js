/******************************************************************************
@Description :   seqDB-16036:  Increment字段参数校验  
@Modify list :   2018-10-23    xiaoni Zhao  Init
******************************************************************************/
function main()
{    
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
   
   var clName = COMMCLNAME + "_16036";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   
   //illegal Increment value
   createAutoIncrement(dbcl, "id1", -2147483648);
   
   createAutoIncrement(dbcl, "id2", 2147483648);
   
   createAutoIncrement(dbcl, "id3", 0);
   
   createAutoIncrement(dbcl, "id4", { "$numberLong" : "-4223372036" });
   
   createAutoIncrement(dbcl, "id5", 123.456);
   
   createAutoIncrement(dbcl, "id6", { $decimal:"123.456" });
   
   createAutoIncrement(dbcl, "id7", null);
   
   createAutoIncrement(dbcl, "id8", true);
   
   //legal Increment value
   dbcl.createAutoIncrement([{ Field : "a1", Increment : -2147483647 },
                             { Field : "a2", Increment : 5 },
                             { Field : "a3", Increment : 2147483647 }]);
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a2_SEQ",
                        "SYS_" + clID + "_a3_SEQ"]; 
   var expSequences =  [{ Increment : -2147483647, StartValue : -1, CurrentValue : -1, 
                          MinValue : { "$numberLong" : "-9223372036854775808" }, "MaxValue" : -1 },
                        { Increment : 5 },
                        { Increment : 2147483647 }];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   //insert records and check
   dbcl.insert( { "a" : 1 } );
   dbcl.insert( { "a" : 2 } );
   
   var rc = dbcl.find();
   var expRecs = [ { "a" : 1, "a1" : -1, "a2" : 1, "a3" : 1 },
                   { "a" : 2, "a1" : -2147483648, "a2" : 6, "a3" : 2147483648 }];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement(dbcl, field, increment)
{
   try
   {
      dbcl.createAutoIncrement({ Field : field, Increment : increment });
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
}

main();