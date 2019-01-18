/***************************************************************************
@Description :seqDB-15941 :指定自增字段、不指定自增字段交替插入记录
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
   
   var clName = COMMCLNAME + "_15941";   
   commDropCL(db, COMMCSNAME, clName, true, true);
  
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, {AutoIncrement:{Field:"id",AcquireSize:10}});
   
   /*需求变更SEQUOIADBMAINSTREAM-4045，用例后续重新实现，已将该用例记录到对应的问题单中
   var expR = [];
   var j=1;
   for(var i=0; i<100; i++)
   {
      if(i%2===1)
      {
         var doc = {a:i,id:i};
         dbcl.insert(doc);
         expR.push(doc);
      }else
      {
         dbcl.insert({a:i});
         expR.push({a:i,id:j});
         j++;
      }
   }
   
   var actR = dbcl.find().sort({a:1});
   checkRec(actR, expR);
   println("---check insert success");*/
   
   commDropCL(db, COMMCSNAME, clName, true, true); 
}
main()