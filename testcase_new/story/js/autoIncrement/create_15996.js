/******************************************************************************
@Description :   seqDB-15996:  创建集合时，创建字段名存在包含关系的自增字段 
@Modify list :   2018-10-18    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_15996";
   
   commDropCL( db, COMMCSNAME, clName );
   
   //create higher layer field
   createAutoIncrement( clName, "a" );
   
   //create lower layer field
   createAutoIncrement( clName, "a.aa.aaa" );
   
   //create same layer field
   createAutoIncrement( clName, "a.aa" );
   
   commDropCL( db, COMMCSNAME, clName );
}

function createAutoIncrement( clName, field )
{
   try
   {
      commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : [ { Field : "a.aa" }, { Field : field } ] } );     
   }catch( e )
   {
      if( e !== -6 )
      {
         throw e;
      }          
   }
}

main();