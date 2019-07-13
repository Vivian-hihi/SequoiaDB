/************************************
*@Description: seqDB-13867:切分表使用Search参数展示访问计划
               seqDB-13869:切分表使用Evaluate参数展示访问计划
               seqDB-13871:切分表使用Estimate参数展示访问计划
               seqDB-13873:切分表使用Expand参数展示访问计划
               seqDB-13878:切分表使用Filter参数展示访问计划
               seqDB-13880:切分表使用Detail参数展示访问计划
               seqDB-13882:切分表使用Run参数展示访问计划
               seqDB-14011:切分表使用Flatten参数展示访问计划
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
   var clName = COMMCLNAME + "13867";
   commDropCL( db, COMMCSNAME, clName, true);
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingKey:{a:1},AutoSplit:true});
   
   var doc = [];
   for(var i=0;i<30000;i++)
   {
      doc.push({a:i,b:i,c:i,d:"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"+i})
   }
   dbcl.insert(doc);
   
   //读取配置文件config.txt中的参数，进行访问计划展示
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
}
main();