/************************************
*@Description: 指定group收集统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.10
*@testlinkCase: seqDB-11620
**************************************/
function main()
{	
   if(commIsStandalone(db))
	{
		println("skip standalone environment");
		return;
	}
	
   var csName = COMMCSNAME + "11620";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
	var csOption = { PageSize: 4096 };
   commCreateCS( db, csName, false, "", csOption );
		
	//create cl	
	var groups = commGetGroups(db);
	var groupName = groups[0][0].GroupName;
	
	var clOption = { Group : groupName };
	var clName = COMMCLNAME + "11620";
	var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
	
	//create index
	commCreateIndex( dbcl, "a", {a : 1}, false )
	
	//insert
	var insertNums = 3000;
	var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
	insertSameDatas( dbcl, insertNums, sameValues );
	
	//check before invoke analyze
	checkStat( db, csName, clName, "a", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains );
	
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains );
	
	println("check result before analyze success!");
	
	//invoke analyze
	var options = {GroupName : groupName};
	analyze( db, options );
	
   //check after analyze
	checkStat( db, csName, clName, "a", true, true );
   
   //check the query explain of master/slave nodes 
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
	
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains );

	//analyze invalid groups
	var options1 = {GroupName : "SYSCoord"};
   checkAnalyzeOtherGroup(options1);
	
	var options2 = {GroupName : "NotExistGroup"};
   checkAnalyzeOtherGroup(options2);
	
	//check catalog
	var options3 = {GroupName : "SYSCatalogGroup"};
	analyze( db, options3 );
	
   println("check result after analyze success!");
	commDropCS( db, csName, true, "drop CS in the end" );
}

function checkAnalyzeOtherGroup( options )
{
   try
   {
      db.analyze( options );
		throw "NEED ANALYZE FAILED";
   }
   catch ( e )
   {
		if( -264 !== e && -154 !== e)
		{
         throw buildException( "check analyze", e, "check analyze", "success", "fail" );
		}
   }
}
main();
