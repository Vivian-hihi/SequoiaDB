/******************************************************************************
@Description :   seqDB-15986:新增嵌套类型的自增字段
@Modify list :   2018-10-16    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_15986";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName );
   
   dbcl.insert( { a : 1, b : 1 } );
  
   dbcl.createAutoIncrement( { Field : "a.aa" } ); 
   
   dbcl.createAutoIncrement( { Field : "b.bb.bbb" } );  
   
   try
   {
      dbcl.createAutoIncrement( { Field : "c.1" } );
      throw "create autoIncrement error!";
   }catch( e )
   {
      if( e !== -6 )
      {
         throw e;
      }
   }
   
   //check autoIncrement
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = [ "SYS_" + clID + "_a.aa_SEQ", "SYS_" + clID + "_b.bb.bbb_SEQ" ];
   var expAotoIncrement = [ { Field : "a.aa", SequenceName : sequenceNames[0] },
                            { Field : "b.bb.bbb", SequenceName : sequenceNames[1] } ];
   checkAutoIncrementonCL( COMMCSNAME, clName, expAotoIncrement );
  
   //check sequence
   for( var i in sequenceNames )
   {
      checkSequence( sequenceNames[i], {} );
   }
   
   commDropCL( db, COMMCSNAME, clName );
}

main();
