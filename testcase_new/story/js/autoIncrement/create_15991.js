/******************************************************************************
@Description :   seqDB-15991: 创建集合时，创建1个自增字段 
@Modify list :   2018-10-17    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_15991";
   var field = "id1";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );

   var clID = getCLID( COMMCSNAME, clName );
   var sequenceName = "SYS_" + clID + "_" + field + "_SEQ";
   var expArr = [ { Field : field, SequenceName : sequenceName } ];
   checkAutoIncrementonCL( COMMCSNAME, clName, expArr );
   
   checkSequence( sequenceName, {} );
   
   dbcl.insert( { id1 : 5, id2 : 6, a : 7 } );

   var rc = dbcl.find();
   var expRecs = [ { "id1" : 5, "id2" : 6, "a" : 7 } ];
   checkRec( rc, expRecs ); 
   
   commDropCL(db, COMMCSNAME, clName);
}

main();