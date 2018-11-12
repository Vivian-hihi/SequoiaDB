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
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   //insert records and check
   var expRecs = [];
   dbcl.insert({ a : 1 });
   expRecs.push({ a : 1, "id1" : 1 });
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs );
   
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
   dbcl.insert({ a : 2 });
   expRecs.push({ a : 2, "id1" : 2 });
   var rc = dbcl.find().sort( { "id1" : 1 } );
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

main();