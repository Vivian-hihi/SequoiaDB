/************************************
*@Description: seqDB-12974:删除普通表cl，清空缓存功能验证
*@author:      zhaoyu
*@createdate:  2018.1.18
*@testlinkCase:seqDB-12974
**************************************/
function main()
{
   var clName1 = COMMCLNAME + "_12974_1";
   var clName2 = COMMCLNAME + "_12974_2";
   var clFullName1 = COMMCSNAME + "." + clName1;
   var clFullName2 = COMMCSNAME + "." + clName2;
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the beginning" ) ;
   
   //获取数据组
   var temp = commGetGroups( db );
   if(commIsStandalone(db) == false){
      var groupName = temp[0][0].GroupName;
   }
   
   //创建2个cl在同一个数据组上
   var CLOption = { Group: groupName };
   var dbcl1 = commCreateCLByOption( db, COMMCSNAME, clName1, CLOption);
   var dbcl2 = commCreateCLByOption( db, COMMCSNAME, clName2, CLOption);
   
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
	
   //执行统计
   analyze( db );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", true, true );
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   
   //cl1、cl2中执行查询
   findConf = {a:sameValues};
	query( dbclPrimary1, findConf, null, null, insertNum );
	query( dbclSlave1, findConf, null, null, insertNum );
	query( dbclPrimary2, findConf, null, null, insertNum );
	query( dbclSlave2, findConf, null, null, insertNum );
	
	//检查访问计划快照
	var expAccessPlan = [{ScanType:"tbscan", IndexName:""},
	                     {ScanType:"tbscan", IndexName:""}];
	var actAccessPlan = getCommonAccessPlans( db, {Collection: clFullName1} );
	checkSnapShotAccessPlans( clFullName1, expAccessPlan, actAccessPlan );
	
	var expAccessPlan = [{ScanType:"tbscan", IndexName:""},
	                     {ScanType:"tbscan", IndexName:""}];
	var actAccessPlan = getCommonAccessPlans( db, {Collection: clFullName2} );
	checkSnapShotAccessPlans( clFullName2, expAccessPlan, actAccessPlan );
   
   //drop cl
   commDropCL( db, COMMCSNAME, clName1) ;
   
   //检查统计信息
   var groups = new Array();
   groups.push( groupName );
   checkStat( db, COMMCSNAME, clName1, "a", false, false, groups );
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   
   //检查访问计划快照
   var expAccessPlan = [];
	var actAccessPlan = getCommonAccessPlans( db, {Collection: clFullName1} );
	checkSnapShotAccessPlans( clFullName1, expAccessPlan, actAccessPlan, groups );
	
	var expAccessPlan = [{ScanType:"tbscan", IndexName:""},
	                     {ScanType:"tbscan", IndexName:""}];
	var actAccessPlan = getCommonAccessPlans( db, {Collection: clFullName2} );
	checkSnapShotAccessPlans( clFullName2, expAccessPlan, actAccessPlan );

   //再次创建cl、创建索引、插入相同数据
   var dbcl1 = commCreateCLByOption( db, COMMCSNAME, clName1, CLOption);
   commCreateIndex( dbcl1, "a", {a:1});
	
   insertDiffDatas( dbcl1, insertNum );
	insertSameDatas( dbcl1, insertNum, sameValues );
	
	//获取主备节点
   var dbclPrimary1 = db1.getCS(COMMCSNAME).getCL(clName1);
   var dbclSlave1 = db2.getCS(COMMCSNAME).getCL(clName1);
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   
   //cl1中执行查询
	query( dbclPrimary1, findConf, null, null, insertNum );
	query( dbclSlave1, findConf, null, null, insertNum );
	
   //检查访问计划快照
   var expAccessPlan = [{ScanType:"ixscan", IndexName:"a"},
                        {ScanType:"ixscan", IndexName:"a"}];
	var actAccessPlan = getCommonAccessPlans( db, {Collection: clFullName1} );
	checkSnapShotAccessPlans( clFullName1, expAccessPlan, actAccessPlan );
	
	var expAccessPlan = [{ScanType:"tbscan", IndexName:""},
	                     {ScanType:"tbscan", IndexName:""}];
	var actAccessPlan = getCommonAccessPlans( db, {Collection: clFullName2} );
	checkSnapShotAccessPlans( clFullName2, expAccessPlan, actAccessPlan );

   //清空环境
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the end" ) ;
   commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the end" ) ;
   db1.close();
   db2.close();
   
 }
 main()