/************************************
*@Description: 将统计信息修改为非法值后加载至缓存 
*@author:      zhaoyu
*@createdate:  2017.11.14
*@testlinkCase:seqDB-11635
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11635";
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
   var db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db1.getCS(COMMCSNAME).getCL(clName);
   var db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db2.getCS(COMMCSNAME).getCL(clName);
	
   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName} );
   
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
   
   //生成默认统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 3 success!");
   
   //手工修改主节点统计信息
   var mcvValues = [{a:8000},{a:9000},{a:9001}];
   var fracs = [50,5000,50];
   var rule = {"$set": {"KeyPattern": {a: -1}, "MCV": {"Values": mcvValues, "Frac": fracs}}};
   updateIndexStateInfo( db, COMMCSNAME, clName, "a", rule );
   
   //统计信息加载至缓存
   try
   {
      db.analyze({Mode:4, Collection: COMMCSNAME + "." + clName});
      throw "NEED_ERR";
   }catch(e){
      if(e !== -264 && e !== -47)
      {
         throw e;
      }
   }
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 4 success!");
   
   //手工修改主节点统计信息
   var mcvValues = [{a:8000},{a:9000},{a:9001}];
   var fracs = [50,5000,50];
   var rule = {"$set": {"KeyPattern": {a:1, b: -1}, "MCV": {"Values": mcvValues, "Frac": fracs}}};
   updateIndexStateInfo( db, COMMCSNAME, clName, "a", rule );
   
   //统计信息加载至缓存
   try
   {
      db.analyze({Mode:4, Collection: COMMCSNAME + "." + clName});
      throw "NEED_ERR";
   }catch(e){
      if(e !== -264 && e !== -47)
      {
         throw e;
      }
   }
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 4 success!");
   
   //再次执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName} );
   
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
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
   db1.close();
   db2.close();
  
 }
 main()
 
 /************************************
*@Description: 手工修改索引统计表
*@author:      liuxiaoxuan
*@createDate:  2017.11.13
**************************************/
function updateIndexStateInfo( db, csName, clName, indexName, rule )
{
	var dataDB = new Array();
	if(commIsStandalone(db))
	{
		dataDB[0] = db;
	}
	else
	{
		var groupNames = commGetCLGroups( db, csName + "." + clName );
		var groupDetail = commGetGroups( db, false, groupNames[0] );
		for(var j = 1; j < groupDetail[0].length; j++)
      {
          var hostName = groupDetail[0][j].HostName;
          var svcName = groupDetail[0][j].svcname;
          dataDB[j-1] = new Sdb(hostName, svcName);
		}
	}		
	
	for(var i in dataDB)
	{
		try
	   {
			 var rec = dataDB[i].SYSSTAT.SYSINDEXSTAT.find().toArray();
			 
		    if(0 < rec.length)
		    {	               
             var matcher = {"$and": [{"CollectionSpace" : csName},
				                         {"Collection" : clName},
												 {"Index" : indexName}]};
												 
		       dataDB[i].SYSSTAT.SYSINDEXSTAT.upsert(rule, matcher);
		    }
		}
		catch(e)
	   {
          throw buildException("modify SYSInfo", e, "modify", "modify success", e);
	   }	
	}

}