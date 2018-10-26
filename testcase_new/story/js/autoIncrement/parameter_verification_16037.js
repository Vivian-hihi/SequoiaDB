/******************************************************************************
@Description :   seqDB-16037:  StartValue字段参数校验 
@Modify list :   2018-10-23    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16037";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   
   //create autoIncrement
   dbcl.createAutoIncrement([{ Field : "a1", StartValue : { "$numberLong" : "9223372036854775809" } },
                             { Field : "a2", StartValue : { "$numberLong" : "-9223372036854775809" }, Increment : -1 },
                             { Field : "a3", StartValue : { "$numberLong" : "9223372036854775807" } },
                             { Field : "a4", StartValue : 5 },
                             { Field : "a5", StartValue : { "$numberLong" : "-9223372036854775808" }, Increment : -1 }]);
   
   createAutoIncrement(dbcl, "id5", 123.4);
   
   createAutoIncrement(dbcl, "id6", { $decimal:"123.456" });
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a2_SEQ",
                        "SYS_" + clID + "_a3_SEQ",
                        "SYS_" + clID + "_a4_SEQ",
                        "SYS_" + clID + "_a5_SEQ"]; 
   var expSequences =  [{ StartValue : { "$numberLong" : "9223372036854775807" }, 
                          CurrentValue : { "$numberLong" : "9223372036854775807" } },
                        { Increment : -1, StartValue : { "$numberLong" : "-9223372036854775808" }, 
                          CurrentValue : { "$numberLong" : "-9223372036854775808" }, MaxValue : -1,
                          MinValue : { "$numberLong" : "-9223372036854775808" } },
                        { StartValue : { "$numberLong" : "9223372036854775807" }, 
                          CurrentValue : { "$numberLong" : "9223372036854775807" } },
                        { StartValue : 5, CurrentValue : 5 },
                        { Increment : -1, StartValue : { "$numberLong" : "-9223372036854775808" }, 
                          CurrentValue : { "$numberLong" : "-9223372036854775808" }, MaxValue : -1,
                          MinValue : { "$numberLong":"-9223372036854775808" } }];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   //insert records
   dbcl.insert( { "a" : 1 } );  
   try
   {
      dbcl.insert( { "a" : 2 } );
   }catch(e)
   {
      if(e !== -325 )
      {
         throw e;
      }
   }
   
   //check records
   var rc = dbcl.find();
   var expRecs = [ { "a" : 1, "a1" : {"$numberLong":"9223372036854775807"}, 
                     "a2" : { "$numberLong" : "-9223372036854775808" }, 
                     "a3" : { "$numberLong" : "9223372036854775807" },
                     "a4" : 5, "a5" : { "$numberLong" : "-9223372036854775808" } }];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement(dbcl, field, startValue)
{
   try
   {
      dbcl.createAutoIncrement({ Field : field, StartValue : startValue });
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
}

main();