/************************************
*@Description: 指定切分表所在cs收集统计信息 
*@author:      liuxiaoxuan
*@createdate:  2017.11.08
*@testlinkCase: seqDB-11609
**************************************/
function main()
{
   var csName = COMMCSNAME + "11609";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
	
	//create CLs
	var clOption1 = {ShardingKey:{a:1}, ShardingType:"hash"};
	var clName1 = COMMCLNAME + "11609_1";
	var dbcl1 = commCreateCLByOption( db, csName, clName1, clOption1, true );

	var clOption2 = {ShardingKey:{a:1}, ShardingType:"range"};
	var clName2 = COMMCLNAME + "11609_2";
	var dbcl2 = commCreateCLByOption( db, csName, clName2, clOption2, true );
	
	commCreateIndex( dbcl1, "bc", {b : 1, c : 1}, false )
	commCreateIndex( dbcl2, "bc", {b : -1, c : -1}, false )

	var insertNums = 3000;
   insertDatas( dbcl1, insertNums );
	insertDatas( dbcl2, insertNums );
	
	//check before invoke analyze
	checkStat( db, csName, clName1, "$shard", false, false );
   checkStat( db, csName, clName1, "bc", false, false );
	
	checkStat( db, csName, clName2, "$shard", false, false );
	checkStat( db, csName, clName2, "bc", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf1 = {a : 9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums}];
	
   var findConf2 = {b : 9000, c : 'test9000'};
   var expExplains2 = [{ScanType:"ixscan", IndexName:"bc", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName1, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName1, findConf2, null, null, expExplains2 );
	checkExplain( db, csName, clName2, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName2, findConf2, null, null, expExplains2 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName1, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName1, findConf2, null, null, expExplains2 );
	checkExplain( db, csName, clName2, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName2, findConf2, null, null, expExplains2 );
	
	println("check result before analyze success!");
   
	//invoke analyze
	var options = {CollectionSpace: csName};
   analyze( db, options );
   
   //check after analyze
	checkStat( db, csName, clName1, "$shard", true, true );
   checkStat( db, csName, clName1, "bc", true, true );
	
	checkStat( db, csName, clName2, "$shard", true, true );
	checkStat( db, csName, clName2, "bc", true, true );
   
   //check the query explain of master/slave nodes 
	var findConf1 = {a : 9000};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
	
   var findConf2 = {b : 9000, c : 'test9000'};
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName1, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName1, findConf2, null, null, expExplains2 );
	checkExplain( db, csName, clName2, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName2, findConf2, null, null, expExplains2 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName1, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName1, findConf2, null, null, expExplains2 );
	checkExplain( db, csName, clName2, findConf1, null, null, expExplains1 );
	checkExplain( db, csName, clName2, findConf2, null, null, expExplains2 );
   
   println("check result after analyze success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
}
main();
