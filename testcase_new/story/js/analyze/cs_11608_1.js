/************************************
*@Description: 指定cs清空统计信息,cs下存在1个/多个cl,且cl存在索引和记录
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11608
**************************************/
function main()
{
   var clName1 = COMMCLNAME + "_11608_1";
   var clName2 = COMMCLNAME + "_11608_2";
   var clName3 = COMMCLNAME + "_11608_3";
   var insertNum = 2000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, clName3, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl1 = commCreateCL( db, COMMCSNAME, clName1);
   var dbcl2 = commCreateCL( db, COMMCSNAME, clName2);
   var dbcl3 = commCreateCL( db, COMMCSNAME, clName3);
   
   //创建索引
   commCreateIndex( dbcl1, "a", {a:1});
   commCreateIndex( dbcl2, "a", {a:1});
   commCreateIndex( dbcl3, "a", {a:1});
   
   //插入记录
	insertDatas( dbcl1, insertNum );
	insertDatas( dbcl2, insertNum );
	insertDatas( dbcl3, insertNum );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   println("check cl:" + clName1 + " before analyze success!");
   
   checkStat( db, COMMCSNAME, clName2, "a", false, false );
   println("check cl:" + clName2 + " before analyze success!");
   
   checkStat( db, COMMCSNAME, clName3, "a", false, false );
   println("check cl:" + clName3 + " before analyze success!");
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains );
   
   //执行统计
   analyze( db, {CollectionSpace: COMMCSNAME} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", true, true );
   println("check cl:" + clName1 + " after analyze success!");
   
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   println("check cl:" + clName2 + " after analyze success!");
   
   checkStat( db, COMMCSNAME, clName3, "a", true, true );
   println("check cl:" + clName3 + " after analyze success!");
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains );

   //truncate 其中一个cl的记录,再次执行统计
   dbcl1.truncate();
   analyze( db, {CollectionSpace: COMMCSNAME} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   println("check cl:" + clName1 + " after truncate cl success!");
   
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   println("check cl:" + clName2 + " after truncate cl success!");
   
   checkStat( db, COMMCSNAME, clName3, "a", true, true );
   println("check cl:" + clName3 + " after truncate cl success!");
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains1 );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains2 );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains2 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains1 );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains2 );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains2 );
   
   //删除其中一个cl中的索引,再次执行统计
   commDropIndex( dbcl3, "a" );
   analyze( db, {CollectionSpace: COMMCSNAME} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName1, "a", false, false );
   println("check cl:" + clName1 + " after drop index success!");
   
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   println("check cl:" + clName2 + " after drop index success!");
   
   checkStat( db, COMMCSNAME, clName3, "a", true, false );
   println("check cl:" + clName3 + " after drop index success!");
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var expExplains3 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains1 );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains2 );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains3 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName1, findConf, null, null, expExplains1 );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains2 );
   checkExplain( db, COMMCSNAME, clName3, findConf, null, null, expExplains3 );
   
   //删除其他2个cl，再次执行统计
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, clName3, true, true,"drop CL in the beginning" ) ;
   analyze( db, {CollectionSpace: COMMCSNAME} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName2, "a", true, true );
   println("check only one cl:" + clName2 + " success!");
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains2 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName2, findConf, null, null, expExplains2 );
   
   //清空环境
   commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the end" ) ;
   commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the end" ) ;
   commDropCL( db, COMMCSNAME, clName3, true, true,"drop CL in the end" ) ;
  
 }
 main()