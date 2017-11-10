/************************************
*@Description: 指定切分cl生成默认统计信息并手工修改统计信息再清空
*@author:      liuxiaoxuan
*@createdate:  2017.11.10
*@testlinkCase: seqDB-11624
**************************************/
function main()
{
	if (commIsStandalone(db))
   {
      println("skip standalone environment");
		return ;
   }
	
   var csName = COMMCSNAME + "11624";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
	
	//create CLs
	var clOption1 = {ShardingKey:{a:1}, ShardingType:"hash"};
	var clName1 = COMMCLNAME + "11624_1";
	var dbcl1 = commCreateCLByOption( db, csName, clName1, clOption1, true );

	var clOption2 = {ShardingKey:{a:1}, ShardingType:"range"};
	var clName2 = COMMCLNAME + "11624_2";
	var dbcl2 = commCreateCLByOption( db, csName, clName2, clOption2, true );
	
	commCreateIndex( dbcl1, "b", {b : 1}, false )
	commCreateIndex( dbcl2, "b", {b : -1}, false )

	//insert datas
	var insertNums = 3000;
	var sameValues = 9000;
	
   insertDiffDatas( dbcl1, insertNums );
	insertSameDatas( dbcl1, insertNums, sameValues );
   insertDiffDatas( dbcl2, insertNums );
	insertSameDatas( dbcl2, insertNums, sameValues );
	
	//check before invoke analyze
	checkStat( db, csName, clName1, "$shard", false, false );
   checkStat( db, csName, clName1, "b", false, false );
	
	checkStat( db, csName, clName2, "$shard", false, false );
	checkStat( db, csName, clName2, "b", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf1 = {a : 9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums}];
	
   var findConf2 = {b : 9000};
   var expExplains2 = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains1 );
	checkExplain( dbcl1, findConf2, null, null, expExplains2 );
	checkExplain( dbcl2, findConf1, null, null, expExplains1 );
	checkExplain( dbcl2, findConf2, null, null, expExplains2 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains1 );
	checkExplain( dbcl1, findConf2, null, null, expExplains2 );
	checkExplain( dbcl2, findConf1, null, null, expExplains1 );
	checkExplain( dbcl2, findConf2, null, null, expExplains2 );
	
	println("check result before analyze success!");
   
	//analyze default statistics infomation 
	var options1 = { Mode : 3, Collection: csName + "." + clName1 };
   analyze( db, options1 );
   var options2 = { Mode : 3, Collection: csName + "." + clName2 };
   analyze( db, options2 );
   
   //check the query explain of master/slave nodes 
	var findConf1 = {a : 9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums}];
	
   var findConf2 = {b : 9000};
   var expExplains2 = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains1 );
	checkExplain( dbcl1, findConf2, null, null, expExplains2 );
	checkExplain( dbcl2, findConf1, null, null, expExplains1 );
	checkExplain( dbcl2, findConf2, null, null, expExplains2 );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains1 );
	checkExplain( dbcl1, findConf2, null, null, expExplains2 );
	checkExplain( dbcl2, findConf1, null, null, expExplains1 );
	checkExplain( dbcl2, findConf2, null, null, expExplains2 );
   
	//modify SYSSTAT info
	var groupName1 = commGetCLGroups( db, csName + "." + clName1 );
	var groupName2 = commGetCLGroups( db, csName + "." + clName2 );
	
	var groups1 = commGetGroups( db, false, groupName1[0] );
	var groups2 = commGetGroups( db, false, groupName2[0] );
	
	var matcher1 = [{a: 0},{a: 1},{a:9000}];
	var matcher2 = [{b: 0},{b: 1},{b:9000}];
	
	modifySYSInfo( groups1, csName, clName1, "$shard", matcher1 );
	modifySYSInfo( groups1, csName, clName1, "b", matcher2 );
	modifySYSInfo( groups2, csName, clName2, "$shard", matcher1 );
	modifySYSInfo( groups2, csName, clName2, "b", matcher2 );
	
	var options1 = { Mode : 4, Collection: csName + "." + clName1 };
   analyze( db, options1 );
   var options2 = { Mode : 4, Collection: csName + "." + clName2 };
   analyze( db, options2 );
		
	//check analyze
	var findConf1 = {a : 9000};
	var findConf2 = {b : 9000};	 
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains );
	checkExplain( dbcl1, findConf2, null, null, expExplains );
	checkExplain( dbcl2, findConf1, null, null, expExplains );
	checkExplain( dbcl2, findConf2, null, null, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains );
	checkExplain( dbcl1, findConf2, null, null, expExplains );
	checkExplain( dbcl2, findConf1, null, null, expExplains );
	checkExplain( dbcl2, findConf2, null, null, expExplains );
   
	//truncate invalidate
	var options = { Mode : 5 };
	analyze( db, options );
	
	//check analyze
	db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains );
	checkExplain( dbcl1, findConf2, null, null, expExplains );
	checkExplain( dbcl2, findConf1, null, null, expExplains );
	checkExplain( dbcl2, findConf2, null, null, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl1, findConf1, null, null, expExplains );
	checkExplain( dbcl1, findConf2, null, null, expExplains );
	checkExplain( dbcl2, findConf1, null, null, expExplains );
	checkExplain( dbcl2, findConf2, null, null, expExplains );
	
   println("check result after analyze success!");
	
	//commDropCS( db, csName, true, "drop CS in the end" );
}

function modifySYSInfo( groups, csName, clName, indexName, matcher )
{
	try
	{	
       for(var j= 1; j < groups[0].length; j++)
       {
          var hostName = groups[0][j].HostName;
          var svcName = groups[0][j].svcname;
          var dataDB = new Sdb(hostName, svcName);
		    var rec = dataDB.SYSSTAT.SYSINDEXSTAT.find().toArray();
		
		    if(0 < rec.length)
		    {
			    dataDB.SYSSTAT.SYSINDEXSTAT.upsert({"$set": {"MCV": {"Values": matcher, "Frac": [50,50,900]}}},
		                                        {"$and": [{"CollectionSpace" : csName},{"Collection" : clName},{"Index" : indexName}]});
		    }
      }	
	}
	catch(e)
	{
      throw buildException("modify SYSInfo", e, "modify", "modify success", e);
	}	
}
main();
