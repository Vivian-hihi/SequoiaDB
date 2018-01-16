/************************************
*@Description: truncate cl清空统计信息、清空缓存功能验证 
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11398
**************************************/
function main()
{
   var clName1 = COMMCLNAME + "_11398_1";
   var clName2 = COMMCLNAME + "_11398_2";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl1 = commCreateCL( db, COMMCSNAME, clName1);
   var dbcl2 = commCreateCL( db, COMMCSNAME, clName2);
   
   //创建索引
   commCreateIndex( dbcl1, "a", {a:1});
   commCreateIndex( dbcl2, "a", {a:1});
   
   //插入记录
	insertDiffDatas( dbcl1, insertNum );
	insertSameDatas( dbcl1, insertNum, sameValues );
	
	insertDiffDatas( dbcl2, insertNum );
	insertSameDatas( dbcl2, insertNum, sameValues );
	
	//获取主备节点
	var db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary1 = db1.getCS(COMMCSNAME).getCL(clName1);
   var dbclPrimary2 = db1.getCS(COMMCSNAME).getCL(clName2);
   var db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave1 = db2.getCS(COMMCSNAME).getCL(clName1);
   var dbclSlave2 = db2.getCS(COMMCSNAME).getCL(clName2);
   
	//检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   checkStat( db, COMMCSNAME, clName2, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName1} );
   analyze( db, {Collection: COMMCSNAME + "." + clName2} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", true, true );
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );

   //执行查询
   querySameWithOutExplain( dbclPrimary1, findConf );
   querySameWithOutExplain( dbclPrimary2, findConf );
   querySameWithOutExplain( dbclSlave1, findConf );
   querySameWithOutExplain( dbclSlave2, findConf );

   //检查访问计划快照信息
   var actAccessPlans1 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName1}).toArray();
   var actAccessPlans2 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName2}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
                                                                           
   var expectAccessPlansCluster = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];                         	
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans1 );
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans2 );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans1 );
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans2 );
   } 
   
   println("check result after analyze success!");
   
   //truncate cl
   dbcl1.truncate();
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   checkStat( db, COMMCSNAME, clName2, "a", true, true );

   //检查访问计划快照信息
   var actAccessPlans1 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName1}).toArray();
   var actAccessPlans2 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName2}).toArray();

   var expectAccessPlans1 = [];	
   var expectAccessPlansStandAlone2 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
                                                                           
   var expectAccessPlansCluster2 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];  

   checkSnapShotAccessPlans( expectAccessPlans1, actAccessPlans1 );
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone2, actAccessPlans2 );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster2, actAccessPlans2 );
   } 
    
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );

   //执行查询
   querySameWithOutExplain( dbclPrimary1, findConf );
   querySameWithOutExplain( dbclPrimary2, findConf );
   querySameWithOutExplain( dbclSlave1, findConf );
   querySameWithOutExplain( dbclSlave2, findConf );

   //检查访问计划快照信息
   var actAccessPlans1 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName1}).toArray();
   var actAccessPlans2 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName2}).toArray();

   var expectAccessPlansStandAlone1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
   var expectAccessPlansStandAlone2 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 4} ];	
                                                                           
   var expectAccessPlansCluster1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];  
   var expectAccessPlansCluster2 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 2},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 2} ];  
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone1, actAccessPlans1 );
      checkSnapShotAccessPlans( expectAccessPlansStandAlone2, actAccessPlans2 );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster1, actAccessPlans1 );
      checkSnapShotAccessPlans( expectAccessPlansCluster2, actAccessPlans2 );
   } 
   
   println("check result after truncate cl success!");
   
   //再次插入相同数据
   insertDiffDatas( dbcl1, insertNum );
	insertSameDatas( dbcl1, insertNum, sameValues );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );
   
   //执行查询
   querySameWithOutExplain( dbclPrimary1, findConf );
   querySameWithOutExplain( dbclPrimary2, findConf );
   querySameWithOutExplain( dbclSlave1, findConf );
   querySameWithOutExplain( dbclSlave2, findConf );

   //检查访问计划快照信息
   var actAccessPlans1 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName1}).toArray();
   var actAccessPlans2 = db.snapshot(11, {Collection : COMMCSNAME + "." + clName2}).toArray();

   var expectAccessPlansStandAlone1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 4} ];	
   var expectAccessPlansStandAlone2 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 6} ];	
                                                                           
   var expectAccessPlansCluster1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 2},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 2} ];  
   var expectAccessPlansCluster2 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 3},
                                   {"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 3} ];  
   if(commIsStandalone(db))
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone1, actAccessPlans1 );
      checkSnapShotAccessPlans( expectAccessPlansStandAlone2, actAccessPlans2 );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster1, actAccessPlans1 );
      checkSnapShotAccessPlans( expectAccessPlansCluster2, actAccessPlans2 );
   } 

   println("check result after create the same index success!");
   
   //清空环境
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the end" ) ;
   commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the end" ) ;
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