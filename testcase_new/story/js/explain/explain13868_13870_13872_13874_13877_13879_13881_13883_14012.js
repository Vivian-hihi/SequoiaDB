/************************************
*@Description: seqDB-13868:主子表使用Search参数展示访问计划
               seqDB-13870:主子表使用Evaluate参数展示访问计划
               seqDB-13872:主子表使用Estimate参数展示访问计划
               seqDB-13874:主子表使用Expand参数展示访问计划
               seqDB-13877:主子表使用SubCollections参数展示访问计划
               seqDB-13879:主子表使用Filter参数展示访问计划
               seqDB-13881:主子表使用Detail参数展示访问计划
               seqDB-13883:主子表使用Run参数展示访问计划
               seqDB-14012:主子表使用Flatten参数展示访问计划
*@author:      zhaoyu
*@createdate:  2019.7.13
*@testlinkCase: seqDB-13867
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("------Deploy is standalone");
      return;
   }
      
   if (commGetGroupsNum(db) < 2)
   {
   	println("Deploy is only one group!");
   	return;
   }
   
   var configPath = "./config.txt";
   var mainCLName = COMMCLNAME + "_maincl_13868";
   var subCLName1 = COMMCLNAME + "_subcl_13868_1";
   var subCLName2 = COMMCLNAME + "_subcl_13868_2";
   commDropCL( db, COMMCSNAME, mainCLName, true);
   commDropCL( db, COMMCSNAME, subCLName1, true);
   commDropCL( db, COMMCSNAME, subCLName2, true);
   var dbcl = commCreateCLByOption( db, COMMCSNAME, mainCLName, {ShardingType:"range", ShardingKey:{a:1}, IsMainCL:true});
   commCreateCL( db, COMMCSNAME, subCLName1 );
   commCreateCL( db, COMMCSNAME, subCLName2 );
   dbcl.attachCL(COMMCSNAME + "." + subCLName1, {LowBound:{a:0}, UpBound:{a:10000}});
   dbcl.attachCL(COMMCSNAME + "." + subCLName2, {LowBound:{a:10000}, UpBound:{a:20000}});
   
   var doc = [];
   for(var i=0;i<20000;i++)
   {
      doc.push({a:i,b:i,c:i,d:"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"+i})
   }
   dbcl.insert(doc);
   
   var file = new File(configPath);
   while(true)
   {
      try
      {
         var explainObj = JSON.parse(file.readLine().split("\n")[0]);
         var explainCursor = dbcl.find({a:{$in:[1,10000]}}).explain(explainObj);
         while(explainCursor.next()){};
         
      }catch(e)
      {
         if(e === -9)
         {
            break;
         }else
         {
            throw e;
         }
      }
   }
   
   //使用SubCollections展示访问计划
   var explainCursor = dbcl.find({a:{$in:[1,10000]}}).explain({SubCollections: COMMCSNAME + "." + subCLName1});
   while(explainCursor.next()){};
  
}
main();