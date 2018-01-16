/************************************
*@Description: 删除索引清空统计信息、查询缓存
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11399
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11399";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   commCreateIndex( dbcl, "b", {b:1});
   
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
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   checkStat( db, COMMCSNAME, clName, "b", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   var findConf = {b:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   checkStat( db, COMMCSNAME, clName, "b", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   //执行查询
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf ); 

   var findConf = {b:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //执行查询
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf ); 

   //检查访问计划快照信息
   var actAccessPlans = db.snapshot(11, {Collection : COMMCSNAME + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2},
                                      {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2}];	
                                                      
   var expectAccessPlansCluster = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1}];	
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   }
   
   println("check result after analyze success!");
   
   //删除索引
   commDropIndex( dbcl, "a" );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, false );
   checkStat( db, COMMCSNAME, clName, "b", true, true );

   //检查访问计划快照信息
   var actAccessPlans = db.snapshot(11, {Collection : COMMCSNAME + "." + clName}).toArray();

   var expectAccessPlans = [];	
   checkSnapShotAccessPlans( expectAccessPlans, actAccessPlans );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var hintConf = {"":"a"};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf, null, hintConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf, null, hintConf);
   checkExplain( actExplains, expExplains );
   
   //执行查询
   querySameWithOutExplain( dbclPrimary, findConf, null, hintConf );
   querySameWithOutExplain( dbclSlave, findConf, null, hintConf ); 

   var findConf = {b:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //执行查询
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf ); 

   //检查访问计划快照信息
   var actAccessPlans = db.snapshot(11, {Collection : COMMCSNAME + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2},
                                      {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2}];	
                                                      
   var expectAccessPlansCluster = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1}];	
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   }
   
   println("check result after drop index success!");
   
   //再次创建相同索引
   commCreateIndex( dbcl, "a", {a:1});
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, false );

   //检查访问计划快照信息
   var actAccessPlans = db.snapshot(11, {Collection : COMMCSNAME + "." + clName}).toArray();

   var expectAccessPlans = [];	
   checkSnapShotAccessPlans( expectAccessPlans, actAccessPlans );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   //执行查询
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf ); 

   var findConf = {b:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //执行查询
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf ); 

   //检查访问计划快照信息
   var actAccessPlans = db.snapshot(11, {Collection : COMMCSNAME + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2},
                                      {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2}];	
                                                      
   var expectAccessPlansCluster = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1}];	
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   } 

   println("check result after create the same index success!");
   
   //清空环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" ) ;
   db1.close();
   db2.close();
  
 }

function querySameWithOutExplain(dbcl, findConf, sortConf, hintConf)
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   
   //执行查询
   var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf);
   while(rc.next())
   {
   }
}
 main()