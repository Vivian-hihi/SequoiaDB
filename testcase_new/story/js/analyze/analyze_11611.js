/************************************
*@Description: 指定普通cl收集统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11611
**************************************/
function main()
{
   var csName = COMMCSNAME + "11611";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
	
	//create CLs
	var clName1 = COMMCLNAME + "11611_1";
	var dbcl1 = commCreateCL( db, csName, clName1 );
	
	var clName2 = COMMCLNAME + "11611_2";
	var dbcl2 = commCreateCL( db, csName, clName2 );

	var clOption = {ShardingKey:{a:1}, ShardingType:"hash"};
	var clName3 = COMMCLNAME + "11611_hash";
	var dbcl3 = commCreateCLByOption( db, csName, clName3, clOption, true );
	
	var clOption = {ShardingKey:{a:1}, ShardingType:"range"};
	var clName4 = COMMCLNAME + "11611_range";
	var dbcl4 = commCreateCLByOption( db, csName, clName4, clOption, true );
	
	var insertNums = 3000;
	// include datas , but no index
   insertDatas( dbcl1, insertNums );
	// include datas and index
	insertDatas( dbcl3, insertNums );
	
	//check before invoke analyze
	checkStat( db, csName, clName1, "", false, false );
   checkStat( db, csName, clName2, "", false, false );
	checkStat( db, csName, clName3, "$shard", false, false );
	checkStat( db, csName, clName4, "$shard", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf = {a : 9000};
	
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:0}];
   var expExplains3 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums}];
	var expExplains4 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:0}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName1, findConf, null, null, expExplains1 )
	checkExplain( db, csName, clName2, findConf, null, null, expExplains2 )
	checkExplain( db, csName, clName3, findConf, null, null, expExplains3 )
	checkExplain( db, csName, clName4, findConf, null, null, expExplains4 )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName1, findConf, null, null, expExplains1 )
	checkExplain( db, csName, clName2, findConf, null, null, expExplains2 )
	checkExplain( db, csName, clName3, findConf, null, null, expExplains3 )
	checkExplain( db, csName, clName4, findConf, null, null, expExplains4 )
	
	println("check result before analyze success!");
   
	//invoke analyze
	var options = [{Collection: csName + "." + clName1},
	               {Collection: csName + "." + clName2},
						{Collection: csName + "." + clName3},
						{Collection: csName + "." + clName4}];
						
	for(var i in options)
	{
		analyze( db, options[i] );
	}
   
   //check after analyze
	checkStat( db, csName, clName1, "", true, false );
   checkStat( db, csName, clName2, "", false, false );
	checkStat( db, csName, clName3, "$shard", true, true );
	checkStat( db, csName, clName4, "$shard", false, false );
   
   //check the query explain of master/slave nodes 
	var findConf = {a : 9000};
	
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:0}];
	var expExplains3 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
	var expExplains4 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:0}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName1, findConf, null, null, expExplains1 )
	checkExplain( db, csName, clName2, findConf, null, null, expExplains2 )
	checkExplain( db, csName, clName3, findConf, null, null, expExplains3 )
	checkExplain( db, csName, clName4, findConf, null, null, expExplains4 )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName1, findConf, null, null, expExplains1 )
	checkExplain( db, csName, clName2, findConf, null, null, expExplains2 )
	checkExplain( db, csName, clName3, findConf, null, null, expExplains3 )
	checkExplain( db, csName, clName4, findConf, null, null, expExplains4 )
	
	//analyze table below SYSSTAT 
	var options1 = {Collection: "SYSSTAT.SYSCOLLECTIONSTAT"};
   checkAnalyzeInvalidResult(options1);
	
	var options2 = {Collection: "SYSSTAT.SYSINDEXSTAT"};
   checkAnalyzeInvalidResult(options2);
	
   println("check result after analyze success!");
}

function checkAnalyzeInvalidResult( options )
{
   try
   {
      db.analyze( options );
		throw "NEED ANALYZE FAILED";
   }
   catch ( e )
   {
		if( -23 !== e )
		{
         throw buildException( "check analyze", e, "check analyze", "success", "fail" );
		}
   }
}
main();
