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
   
   dbcl.createAutoIncrement({ Field : "a2", MaxValue : { "$numberLong" : "9223372036854775809" } });
   
   create(dbcl, "a3", { "$numberLong" : "-9223372036854775809" }, { "$numberLong" : "-9223372036854775860" });
   
   create(dbcl, "a4", { "$numberLong" : "-9223372036854775808" }, { "$numberLong" : "-9223372036854775860" } );
   
   dbcl.createAutoIncrement({ Field : "a5", MaxValue : 5000 });
   
   create(dbcl, "a6", 2000, 3000);
   
   create(dbcl, "a7", 2000, 2000);
   
   create(dbcl, "a8", 20.55);
   
   create(dbcl, "a9", { $decimal:"123.456" });
   
   var expRecs = [];
   for(var i = 0; i < 50; i++)
   {
      dbcl.insert({ a : i });
      expRecs.push({ "a" : i, "a1" : 1 + i, "a2" : 1 + i, "a5" : 1 + i });
   }
   
   var rc = dbcl.find().sort({ "a1" : 1 });
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function create(dbcl, field, maxValue, minValue)
{
    if(minValue == undefined)
    {
       minValue = 1;
    }
    try
    {
       dbcl.createAutoIncrement({ Field : field, MaxValue : maxValue, MinValue : minValue, CacheSize : 1, AcquireSize : 1 });
    }catch(e)
    {
       if(e !== -6)
       {
           throw "create error!";
       }
    }
}

main();