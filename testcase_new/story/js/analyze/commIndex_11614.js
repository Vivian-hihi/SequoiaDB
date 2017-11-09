/************************************
*@Description: 指定普通索引收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.9
*@testlinkCase:seqDB-11614
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11614";
   var insertNum = 2000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   
   //插入记录
	insertDatas( dbcl, insertNum );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   println("check result after analyze success!");
   
   //删除索引
   commDropIndex( dbcl, "a" );
   
   //指定不存在的索引执行统计信息
   /*
   try
   {
      db.analyze({Collection: COMMCSNAME + "." + clName, Index: "a"});
      throw "NEED_ERR";
   }catch(e)
   {
      if(e !== -264)
      {
         throw e;
      }
   }
   */
   
   //不指定cl但指定索引收集统计信息
   try
   {
      db.analyze({Index: "a"});
      throw "NEED_ERR";
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
  
 }
 main()