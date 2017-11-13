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
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
	//获取主备节点
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(COMMCSNAME).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(COMMCSNAME).getCL(clName);
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a"} );
   
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
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