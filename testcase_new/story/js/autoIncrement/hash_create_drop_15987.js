/******************************************************************************
@Description :   seqDB-15987:hash分区表上创建/删除自增字段 
@Modify list :   2018-10-15    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   var dataGroupNames = getDataGroupNames();
   if(commIsStandalone( db ) || dataGroupNames.length <2)
   {
      println("Deploy is standalone or only one group");
          return;
   } 
   
   var clName = COMMCLNAME + "_15987";
   var field = "id1";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { Group:dataGroupNames[0], ShardingKey : { a : 1 }, ShardingType : "hash" } );
   
   dbcl.insert( { a : 1 } );
   
   dbcl.split( dataGroupNames[0], dataGroupNames[1], 50 );
   
   dbcl.createAutoIncrement( { Field : field } );
   
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceName = "SYS_" + clID + "_" + field + "_SEQ";
   var expArr = [ { Field : field, SequenceName : sequenceName } ];
   checkAutoIncrementonCL( COMMCSNAME, clName, expArr );
   checkSequence( sequenceName, {} );
      
   dbcl.insert( { a : 2 } );
   
   var rc = dbcl.find().sort( { field : 1 } );
   var expRecs = [ { "a" : 1 }, { "id1" : 1, "a" : 2 } ];
   checkRec( rc, expRecs );
   
   dbcl.dropAutoIncrement( "id1" ); 
   
   var cursor = db.snapshot( 8, { Name : COMMCSNAME + "." + clName } );
   if( cursor.current().toObj().AutoIncrement.length !== 0 )
   {
      throw "drop autoIncrement failed!";
   }
   
   commDropCL( db, COMMCSNAME, clName );
}

main();