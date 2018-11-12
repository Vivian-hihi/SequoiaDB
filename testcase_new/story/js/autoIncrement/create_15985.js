/******************************************************************************
@Description :   seqDB-15985:新增自增字段与已存在的自增字段存在包含关系
@Modify list :   2018-10-16    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_15985";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "a.aa" } } );
   
   //create lower layer field
   createAutoIncrement( dbcl, "a.aa.aa" );
   
   //create higher layer field
   createAutoIncrement( dbcl, "a" );
   
   //create same layer field
   dbcl.createAutoIncrement({ Field : "a.bb" });
   
   //check autoIncrement 
   var clID = getCLID( COMMCSNAME, clName );
   var sequenceNames = ["SYS_" + clID + "_a.aa_SEQ",
                        "SYS_" + clID + "_a.bb_SEQ"]; 
   var expIncrements = [{ Field : "a.aa", SequenceName : sequenceNames[0] },
                        { Field : "a.bb", SequenceName : sequenceNames[1] }];
   checkAutoIncrementonCL( COMMCSNAME, clName, expIncrements );
   
   //check sequence
   for( var i in sequenceNames )
   {
      checkSequence(sequenceNames[i], {} );
   }
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement( dbcl, field )
{
   try
   {
      dbcl.createAutoIncrement( { Field : field } );   
      throw "create autoIncrement error!";      
   }catch( e )
   {
      if( e !== -332 )
      {
         throw e;
      }          
   }
}

main();