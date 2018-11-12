/******************************************************************************
@Description :   seqDB-16035:  Field字段参数校验 
@Modify list :   2018-10-23    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_16035";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   //illegal Field value
   create(dbcl);
   
   create(dbcl, "$id2");
   
   create(dbcl, " id3");
   
   create(dbcl, "5");
   
   //legal Field value
   dbcl.createAutoIncrement({ Field : "id6" });
   
   dbcl.createAutoIncrement({ Field : "id$7" });
   
   dbcl.createAutoIncrement({ Field : "id 8" });
   
   //check autoIncrement count
   var cursor = db.snapshot(8, { Name : COMMCSNAME + "." + clName });
   if( cursor.current().toObj().AutoIncrement.length !== 4)
   {
      throw "autoIncrement count error!";
   }
   
   //check autoIncrement value
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_id1_SEQ",
                        "SYS_" + clID + "_id6_SEQ",
                        "SYS_" + clID + "_id$7_SEQ",
                        "SYS_" + clID + "_id 8_SEQ"]; 
   var expIncrements = [{ Field : "id1", SequenceName : sequenceNames[0] },
                        { Field : "id6", SequenceName : sequenceNames[1] },
                        { Field : "id$7", SequenceName : sequenceNames[2] },
                        { Field : "id 8", SequenceName : sequenceNames[3] }];
   checkAutoIncrementonCL( COMMCSNAME, clName, expIncrements );
    
   //insert records and check
   dbcl.insert( { "a" : 1 } );
   
   var rc = dbcl.find();
   var expRecs = [ { "a" : 1, "id1" : 1, "id6" : 1, "id$7" : 1, "id 8" : 1 }];
   checkRec( rc, expRecs );
   
   commDropCL( db, COMMCSNAME, clName );
}

function create(dbcl, field)
{
   try
   {
      if(field == undefined)
      {
         dbcl.createAutoIncrement({ StartValue : 3 });   
         throw "create autoIncrement error!";
      }else
      {
         dbcl.createAutoIncrement({ Field : field, StartValue : 3 });
         throw "create autoIncrement error!";
      }
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
}

main();