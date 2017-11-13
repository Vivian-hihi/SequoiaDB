/************************************
*@Description:  不指定cl和index生成默认统计信息 
*@author:      liuxiaoxuan
*@createdate:  2017.11.10
*@testlinkCase: seqDB-11622
**************************************/
function main()
{	
   if (commIsStandalone(db))
   {
      println("skip standalone environment");
		return ;
   }
	
   var csName = COMMCSNAME + "11622";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
	//create cl	
	var clName = COMMCLNAME + "11622";
	var dbcl = commCreateCL( db, csName, clName );
	
	//get master/slave datanode
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(csName).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(csName).getCL(clName);
	
	//create index
	commCreateIndex( dbcl, "a", {a : 1}, false )
	
	//insert datas
	var insertNums = 3000;
	var sameValues = 9000;
	insertDiffDatas( dbcl, insertNums );
	insertSameDatas( dbcl, insertNums, sameValues )
	
	//check before invoke analyze
	checkStat( db, csName, clName, "a", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");
	
	//analyze with Mode:3
	var options1 = { Mode : 3 };
	checkAnalyze( options1 );
	
	var options2 = { Mode : 3, CollectionSpace: csName };
	checkAnalyze( options2 );
	
	var groupName = commGetCLGroups( db, csName + "." + clName );
	var options2 = { Mode : 3, GroupName: groupName[0] };
	checkAnalyze( options2 );
	
	var groupArray = commGetGroups( db, false, groupName[0] );
	var priNode = groupArray[0][0].PrimaryNode;
	var options3 = { Mode : 3, NodeID: priNode };
	checkAnalyze( options3 );
	
   //check after analyze
	checkStat( db, csName, clName, "a", false, false );
   
   //check the query explain of master/slave nodes 
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   println("check result after analyze success!");
}

function checkAnalyze( options )
{  
   try
   {
      db.analyze( options );
		throw "NEED ANALYZE FAILED";
   }
   catch(e)
   {
		if(-6 !== e)
		{
			throw buildException("check analyze", e, "analyze", "success", "fail");
		}
   }
}

main();
