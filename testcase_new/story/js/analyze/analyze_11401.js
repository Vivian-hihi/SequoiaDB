/************************************
*@Description: alter cl更新统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11401
**************************************/
function main()
{
	if(commIsStandalone(db))
	{
		println("skip standalone environment");
		return;
	}
	
   var csName = COMMCSNAME + "11401";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
	//create cl	
	var clName = COMMCLNAME + "11401";
	var dbcl = commCreateCL( db, csName, clName );
	
	//create index
	commCreateIndex( dbcl, "b", {b : 1}, false )
	
	//insert
	var insertNums = 3000;
	var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
	insertSameDatas( dbcl, insertNums, sameValues );
	
	//check before invoke analyze
	checkStat( db, csName, clName, "b", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf = {b : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
	println("check result before analyze success!");
	
	//invoke analyze
	var options = {CollectionSpace: csName};
	analyze( db, options );
	
   //check after analyze
	checkStat( db, csName, clName, "b", true, true );
   
   //check the query explain of master/slave nodes 
	var findConf1 = {b : 9000};
	var findConf2 = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
	
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf1, null, null, expExplains )
	checkExplain( dbcl, findConf2, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf1, null, null, expExplains )
	checkExplain( dbcl, findConf2, null, null, expExplains )

	//alter cl
	alterCL( dbcl );
	
	//check after alter
	var findConf1 = {b : 9000};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
	
	var findConf2 = {a : 9000};
   var expExplains2 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums}];
	
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf1, null, null, expExplains1 )
	checkExplain( dbcl, findConf2, null, null, expExplains2 )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf1, null, null, expExplains1 )
	checkExplain( dbcl, findConf2, null, null, expExplains2 )
	
   println("check result after analyze success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
}

function alterCL( dbcl )
{
   try
   {
      dbcl.alter( {ShardingKey: {a : 1}, ShardingType: "hash"} );
   }
	catch(e)
   {
      throw buildException("alter CL", e, "alter", "alter success", e);
   }
}
main();
