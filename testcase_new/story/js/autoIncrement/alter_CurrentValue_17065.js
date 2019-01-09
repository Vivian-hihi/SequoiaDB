/******************************************************************************
@Description :   seqDB-17065:指定Increment值创建自增字段，修改CurrentValue
@Modify list :   2019-01-08    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
   
   var clName = COMMCLNAME + "_17065";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   dbcl.createAutoIncrement({Field : "a"});
   
   try
   {
      dbcl.setAttributes({ AutoIncrement : { Field : "a", CurrentValue : -20 } });
   }
   catch(e)
   {
      if(e != -6 )
      {
         throw "alter error!";
      }
   }
   
   dbcl.setAttributes({ AutoIncrement : { Field : "a", CurrentValue : 20 } }); 
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceName = "SYS_" + clID + "_a_SEQ";
   var expSequence =  {CurrentValue : 20};
   checkSequence(sequenceName, expSequence);
   var expRecs = [];
   for(var i = 0; i < 100; i++)
   {
      dbcl.insert({"q" : i});
      expRecs.push({"q" : i, "a" : 21 + i});
   }
   var rc = dbcl.find();
   checkRec( rc, expRecs );
   
   dbcl.dropAutoIncrement("a");
   dbcl.createAutoIncrement({Field : "a", Increment : -2});
   
   try
   {
      dbcl.setAttributes({ AutoIncrement : { Field : "a", CurrentValue : 2 } });
   }
   catch(e)
   {
      if(e != -6 )
      {
         throw "alter error!";
      }
   }
   
   dbcl.setAttributes({ AutoIncrement : { Field : "a", CurrentValue : -50 } });
   var expSequence =  {"Increment" : -2, "CurrentValue" : -50, "MaxValue":-1,"MinValue":{"$numberLong":"-9223372036854775808"},"StartValue":-1};
   checkSequence(sequenceName, expSequence);
   for(var i = 0; i < 100; i++)
   {
      dbcl.insert({"q" : i});
      expRecs.push({"q" : i, "a" : -52 - 2*i});
   }
   var rc = dbcl.find();
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}
main();