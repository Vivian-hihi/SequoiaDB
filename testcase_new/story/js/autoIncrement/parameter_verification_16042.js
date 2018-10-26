/******************************************************************************
@Description :   seqDB-16042:  Generated字段参数校验  
@Modify list :   2018-10-24    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_16042";
   
   commDropCL( db, COMMCSNAME, clName );
   
   //create autoIncrement Generated "default"
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   //create autoIncrement Generated "always"
   dbcl.createAutoIncrement({ Field : "id2", Generated : "always" });
   
   //create autoIncrement Generated "strict"
   dbcl.createAutoIncrement({ Field : "id3", Generated : "strict" });
   
   //create autoIncrement Generated "a"
   try
   {
      dbcl.createAutoIncrement({ Field : "id3", Generated : "a" });
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      } 
   }
   
   //check autoIncrement
   var cursor = db.snapshot(8, { Name : COMMCSNAME + "." + clName });
   var generated = ["strict", "always", "default"];
   for(var i in generated)
   {
      if( cursor.current().toObj().AutoIncrement.length !== 3)
      {
         throw "autoIncrement count error!";
      }
      if( cursor.current().toObj().AutoIncrement[i].Generated !== generated[i])
      {
         throw "create failed!";
      }   
   }
   
   //insert records and check
   dbcl.insert({ "q" : 1, "id2" : 5, "id3" : 5 });
   dbcl.insert({ "q" : 2, "id1" : 5});
   try
   {
      dbcl.insert({ "q" : 2, "id1" : 2, "id2" : 5, "id3" : "f" });     
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
   var rc = dbcl.find().sort( { "id1" : 1 } );
   var expRecs = [ { "q" : 1, "id1" : 1, "id2" : 1, "id3" : 5 },
                   { "q" : 2, "id1" : 5, "id2" : 2, "id3" : 1 }];
   checkRec( rc, expRecs );
  
   commDropCL( db, COMMCSNAME, clName );
}
main();