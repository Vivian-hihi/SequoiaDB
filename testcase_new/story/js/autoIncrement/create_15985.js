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
   createAutoIncrement( dbcl, "a.aa.aa" )
   
   //create higher layer field
   createAutoIncrement( dbcl, "a" )
   
   //create same layer field
   createAutoIncrement( dbcl, "a.aa" )
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement( dbcl, field )
{
   try
   {
      dbcl.createAutoIncrement( { Field : field } );      
   }catch( e )
   {
      if( e !== -332 )
      {
         throw e;
      }          
   }
}

main();