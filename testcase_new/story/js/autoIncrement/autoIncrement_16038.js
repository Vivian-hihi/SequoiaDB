/******************************************************************************
@Description :   seqDB-16038:  MinValue字段参数校验 
@Modify list :   2018-10-23    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
   
   var clName = COMMCLNAME + "_16038";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   
   dbcl.createAutoIncrement([{Field : "a" },
                             { Field : "a1", MinValue : { "$numberLong" : "-9223372036854775809" } },
                             { Field : "a2", MinValue : 5, StartValue : 10 },
                             { Field : "a3", MinValue : { "$numberLong" : "-9223372036854775808" } }]);
   
   createAutoIncrement(dbcl, {Field : "a4", MinValue : { "$numberLong" : "9223372036854775809" }, StartValue : { "$numberLong" : "9223372036854775809" }});
   
   createAutoIncrement(dbcl, {Field : "a5", MinValue : { "$numberLong" : "9223372036854775807" }, StartValue : { "$numberLong" : "9223372036854775809" }});
   
   createAutoIncrement(dbcl, {Field : "a6", MinValue : 20, StartValue : 20, MaxValue : 20});
   
   createAutoIncrement(dbcl, {Field : "a7", MinValue : 25, MaxValue : 20});
   
   createAutoIncrement(dbcl, {Field : "a8", MinValue : 123.4});
   
   createAutoIncrement(dbcl, {Field : "a9", MinValue : { $decimal:"123.456" }});
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a_SEQ",
                        "SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a2_SEQ",
                        "SYS_" + clID + "_a3_SEQ"]; 
   var expSequences =  [{},
                        { MinValue : { "$numberLong" : "-9223372036854775808" } },
                        { MinValue : 5, CurrentValue : 10, StartValue : 10 },
                        { MinValue : { "$numberLong":"-9223372036854775808" } }];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   dbcl.insert( { "q" : 1 } );
  
   var rc = dbcl.find();
   var expRecs = [ { "q" : 1, "a" : 1, "a1" : 1, "a2" : 10, "a3" : 1 } ];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement(dbcl, options)
{
   try
   {
      dbcl.createAutoIncrement(options);
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