/***************************************************************************
@Description : seqDB-15942:自增字段为唯一索引，交替插入记录
@Modify list :
              2018-10-16  zhaoyu  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
	  return;
   }
   
   var clName = COMMCLNAME + "_15942";   
   commDropCL(db, COMMCSNAME, clName, true, true);
  
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, {AutoIncrement:{Field:"id",AcquireSize:10}});
   commCreateIndex( dbcl, "id", {id:-1}, true);
   
   /*需求变更SEQUOIADBMAINSTREAM-4045，用例后续重新实现，已将该用例记录到对应的问题单中
   dbcl.insert({a:1,id:1});
   try
   {
      dbcl.insert({a:2});
      throw "NEED_ERR";
   }catch(e)
   {
      if( e !== -38)
      {
         throw e;
      }
   }*/
   
   commDropCL(db, COMMCSNAME, clName, true, true); 
}
main()