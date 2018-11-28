/******************************************************************************
@Description :   seqDB-16332:  修改不存在的自增字段的属性 
@Modify list :   2018-11-12    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }  
   
   var clName = COMMCLNAME + "_16332";
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { AutoIncrement : { Field : "a1" } } );     
   
   try
   {
      dbcl.setAttributes({ Field : "a2", Increment : 3 });
      throw "alter error!";  
   }catch(e)
   {
      if(e !== -6)
      {
         throw "drop autoIncrement error!";
      }
   }
   
   commDropCL( db, COMMCSNAME, clName );
}
main();