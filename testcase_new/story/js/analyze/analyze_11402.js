/************************************
*@Description: split cl更新统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11402
**************************************/
function main()
{
	if(commIsStandalone(db))
	{
		println("skip standalone environment");
		return;
	}
	
	if(2 > commGetGroupsNum(db))
	{
		println("group nums less than 2");
		return;
	}
	
   var csName = COMMCSNAME + "11402";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
	//create cl
	var groups = commGetGroups(db);
	var srcGroupName = groups[0][0].GroupName;
	var destGroupName = groups[1][0].GroupName;
	println("srcGroupName: " + srcGroupName + " destGroupName: " + destGroupName);
	
	var clOption = {ShardingKey:{a:1}, ShardingType:"hash", Group : srcGroupName};
	var clName = COMMCLNAME + "11402";
	var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
	
	//create index
	commCreateIndex( dbcl, "ac", {a : 1 , c : 1}, false )
	
	//insert
	var insertNums = 3000;
	var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
	insertSameDatas( dbcl, insertNums, sameValues );
	
	//check before invoke analyze
	checkStat( db, csName, clName, "ac", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf = {a : 9000, c : "test9000"};
   var expExplains = [{ScanType:"ixscan", IndexName:"ac", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
	println("check result before analyze success!");
	
	//invoke analyze
	var options = {Collection: csName + "." + clName};
	analyze( db, options );
	
   //check after analyze
	checkStat( db, csName, clName, "ac", true, true );
   
   //check the query explain of master/slave nodes 
	var findConf = {a : 9000, c : "test9000"};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
  
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
	//split cl
	splitCL(dbcl, srcGroupName, destGroupName);
	
	//check after split
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
   println("check result after analyze success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
}

function splitCL( dbcl, srcGroup, destGroup )
{
   try
   {
      dbcl.split( srcGroup, destGroup, 50 );
   }
	catch(e)
   {
      throw buildException("split CL", e, "split", "split success", e);
   }
}
main();
