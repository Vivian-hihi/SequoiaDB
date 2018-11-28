/******************************************************************************
@Description :   seqDB-16333:  删除不存在的自增字段 
@Modify list :   2018-11-12    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16333";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "a1" } } );     
   
   try
   {
      dbcl.dropAutoIncrement( "b1" );
      throw "drop error!";  
   }catch(e)
   {
      if(e !== -333)
      {
         throw "drop autoIncrement error!";
      }
   }
   
   commDropCL( db, COMMCSNAME, clName );
}
main();