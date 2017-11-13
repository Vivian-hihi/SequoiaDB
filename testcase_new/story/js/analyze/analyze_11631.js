/************************************
*@Description:  将未被修改的统计信息重新加载至缓存 
*@author:      liuxiaoxuan
*@createdate:  2017.11.11
*@testlinkCase: seqDB-11631
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11631";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
	//create cl	
	var clName = COMMCLNAME + "11631";
	var dbcl = commCreateCL( db, csName, clName );
	
	//get master/slave datanode
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(csName).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(csName).getCL(clName);
	
	//insert datas
	var insertNums = 3000;
	var sameValues = 9000;
	
   insertDiffDatas( dbcl, insertNums );
	insertSameDatas( dbcl, insertNums, sameValues );
	
	//create index
	commCreateIndex( dbcl, "a", {a : 1}, false )
	
	//check before analyze success
	checkStat( db, csName, clName, "a", false, false );
	
	//check the query explain before analyze
	var findConf = {a : 9000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain(actExplains, expExplains);
	
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain(actExplains, expExplains);
	
	println("check result before default analyze !");
	
	//invoke analyze
	var options = { Mode : 3, Collection : csName + "." + clName };
   analyze( db, options );
	
	//check after analyze success
	checkStat( db, csName, clName, "a", true, false );
	
	//check the query explain after analyze
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result after default analyze !");
	
	// modify SYSSTAT info 
	var groupName = commGetCLGroups( db, csName + "." + clName );
	var groups = commGetGroups( db, false, groupName[0] );
	
	var mcvValues = [{a: 0},{a: 1},{a:9000}];
	var fracs = [500,500,9000];
	updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );
	
	// reload analyze again
	var options = { Mode : 4, Collection : csName + "." + clName };
   analyze( db, options );
	
	//check after reload analyze success
	checkStat( db, csName, clName, "a", true, true );
	
	//check the query explain after analyze
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result after reload analyze success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
}


main();
