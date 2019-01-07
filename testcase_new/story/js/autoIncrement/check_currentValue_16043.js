/******************************************************************************
@Description :   seqDB-16043:  CurrentValue字段参数校验 
@Modify list :   2018-12-25    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
   
   var clName = COMMCLNAME + "_16043";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   dbcl.createAutoIncrement([{Field : "a" }, { Field : "a1" }, { Field : "a2", Increment : -1}, { Field : "a3"},
                             { Field : "a4"}, { Field : "a5", Increment : -1 }, { Field : "a6" }]);
   
   dbcl.setAttributes({ AutoIncrement : { Field : "a3", CurrentValue : 10 } });
   
   dbcl.insert( { "q" : 1 } );
   
   var rc = dbcl.find();
   var expRecs = [{"q" : 1, "a" : 1, "a1" : 1, "a2" : -1, "a3" : 11, "a4" : 1, "a5" : -1, "a6" : 1}];
   checkRec( rc, expRecs );
   
   dbcl.setAttributes({ AutoIncrement : { Field : "a" } });
   dbcl.setAttributes({ AutoIncrement : { Field : "a1", CurrentValue : { "$numberLong" : "9223372036854775807" } } });
   //修改CurrentValue的同时修改MinValue才能通过，已提单，单号为SEQUOIADBMAINSTREAM-4060
   dbcl.setAttributes({ AutoIncrement : { Field : "a2", CurrentValue : { "$numberLong" : "-9223372036854775808" }, MinValue : { "$numberLong" : "-9223372036854775808" } } });
   
   dbcl.setAttributes({ AutoIncrement : { Field : "a4", CurrentValue : { "$numberLong" : "9223372036854775809" } } });  
    //修改CurrentValue的同时修改MinValue才能通过，已提单，单号为SEQUOIADBMAINSTREAM-4060
   dbcl.setAttributes({ AutoIncrement : { Field : "a5", CurrentValue : { "$numberLong" : "-9223372036854775809" }, MinValue : { "$numberLong" : "-9223372036854775808" } } });

   try
   {
      dbcl.setAttributes({ AutoIncrement : { Field : "a6", CurrentValue : "a" } });
      throw "setAttributes error!";
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a_SEQ",
                        "SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a2_SEQ",
                        "SYS_" + clID + "_a3_SEQ",
                        "SYS_" + clID + "_a4_SEQ",
                        "SYS_" + clID + "_a5_SEQ",
                        "SYS_" + clID + "_a6_SEQ"]; 
   var expSequences =  [{ CurrentValue : 1001 },
                        { CurrentValue : { "$numberLong" : "9223372036854775807" } },
                        { CurrentValue : { "$numberLong" : "-9223372036854775808" }, Increment : -1, MinValue : { "$numberLong" : "-9223372036854775808" }, MaxValue : -1, "StartValue":-1 },
                        { CurrentValue : 1010 },
                        { CurrentValue : { "$numberLong" : "9223372036854775807" } },
                        { CurrentValue : { "$numberLong" : "-9223372036854775808" },Increment : -1, MinValue : { "$numberLong" : "-9223372036854775808" }, MaxValue : -1,"StartValue":-1 },
                        { CurrentValue : 1001 }];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   try
   {
       dbcl.insert( { "q" : 2 } );
       throw "insert ERROR";
   }catch(e)
   {
       if(e !== -325)
       {
           throw e;
       }
   }
   
   dbcl.dropAutoIncrement(["a2", "a4", "a5"]);
   
   //检查_exceeded变量是否复位
   dbcl.setAttributes({ AutoIncrement : { Field : "a1", CurrentValue : 4 } });
   
   dbcl.insert({"q" : 3});
   
   var rc = dbcl.find();
   var expRecs = [{"q" : 1, "a" : 1, "a1" : 1, "a2" : -1, "a3" : 11, "a4" : 1, "a5" : -1, "a6" : 1},
                  {"q" : 3, "a" : 1002, "a1" : 5, "a3" : 12, "a6" : 2}];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();