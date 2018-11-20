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
   create(dbcl, "id1", -2147483648);
   
   create(dbcl, "id2", 2147483648);
   
   create(dbcl, "id3", 0);
   
   create(dbcl, "id4", { "$numberLong" : "-4223372036" });
   
   create(dbcl, "id5", 123.456);
   
   create(dbcl, "id6", { $decimal:"123.456" });
   
   create(dbcl, "id7", null);
   
   create(dbcl, "id8", true);
   
   //legal Increment value
   dbcl.createAutoIncrement([{ Field : "a" },
                             { Field : "a1", Increment : -2147483647 },
                             { Field : "a2", Increment : 5 },
                             { Field : "a3", Increment : 2147483647 }]);
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a_SEQ",
                        "SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a2_SEQ",
                        "SYS_" + clID + "_a3_SEQ"]; 
   var expSequences =  [{ Increment : 1 },
                        { Increment : -2147483647, StartValue : -1, CurrentValue : -1, 
                          MinValue : { "$numberLong" : "-9223372036854775808" }, "MaxValue" : -1 },
                        { Increment : 5 },
                        { Increment : 2147483647 }];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   //insert records and check
   dbcl.insert( { "q" : 1 } );
   dbcl.insert( { "q" : 2 } );
   
   var rc = dbcl.find();
   var expRecs = [ { "q" : 1, "a" : 1, "a1" : -1, "a2" : 1, "a3" : 1 },
                   { "q" : 2, "a" : 2, "a1" : -2147483648, "a2" : 6, "a3" : 2147483648 }];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function create(dbcl, field, increment)
{
   try
   {
      dbcl.createAutoIncrement({ Field : field, Increment : increment });
      throw "create autoIncrement error!";
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
}

main();