/******************************************************************************
@Description :   seqDB-15984:  新增自增字段与已存在的自增字段同名
@Modify list :   2018-10-16    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_15984";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "id1" } } );
   
   //create same autoIncrement field name
   try
   {
      dbcl.createAutoIncrement( { Field : "id1" } );      
   }catch( e )
   {
      if( e !== -332 )
      {
         throw e;
      }          
   }
   
   commDropCL( db, COMMCSNAME, clName );
}

main();