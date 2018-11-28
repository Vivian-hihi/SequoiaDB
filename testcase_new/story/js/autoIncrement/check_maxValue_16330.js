/******************************************************************************
@Description :   seqDB-16330:  MaxValue字段参数校验  
@Modify list :   2018-11-13    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16330";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "a1" } } );
   
   create(dbcl, {Field : "a2", MaxValue : 2000, MinValue : 3000});
   
   create(dbcl, {Field : "a3", MaxValue : 2000, MinValue : 2000});
   
   dbcl.createAutoIncrement({ Field : "a4", MaxValue : 5000 });
   
   create(dbcl, {Field : "a5", MaxValue : { "$numberLong" : "-9223372036854775808" }, MinValue : { "$numberLong" : "-9223372036854775860" }});
   
   create(dbcl, {Field : "a6", MaxValue : { "$numberLong" : "-9223372036854775809" }, MinValue : { "$numberLong" : "-9223372036854775860" }});
   
   dbcl.createAutoIncrement({ Field : "a7", MaxValue : { "$numberLong" : "9223372036854775807" } });
   
   dbcl.createAutoIncrement({ Field : "a8", MaxValue : { "$numberLong" : "9223372036854775808" } });
   
   create(dbcl, {Field : "a8", MaxValue : 20.55});
   
   create(dbcl, {Field : "a9", MaxValue : { $decimal:"123.456" }});
   
   //check Sequence
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a1_SEQ",
                        "SYS_" + clID + "_a4_SEQ",
                        "SYS_" + clID + "_a7_SEQ",
                        "SYS_" + clID + "_a8_SEQ"]; 
   var expSequences =  [{},
                        { MaxValue : 5000 },
                        { MaxValue : { "$numberLong" : "9223372036854775807" }},
                        { MaxValue : { "$numberLong" : "9223372036854775807" }}];
   for(var i in sequenceNames)
   {
       checkSequence(sequenceNames[i], expSequences[i]);
   }
   
   var expRecs = [];
   for(var i = 0; i < 50; i++)
   {
      dbcl.insert({ a : i });
      expRecs.push({ "a" : i, "a1" : 1 + i, "a4" : 1 + i, "a7" : 1 + i, "a8" : 1 + i });
   }
   
   var rc = dbcl.find().sort({ "a1" : 1 });
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function create(dbcl, options)
{
    try
    {
       dbcl.createAutoIncrement(options);
    }catch(e)
    {
       if(e !== -6)
       {
           throw "create error!";
       }
    }
}

main();