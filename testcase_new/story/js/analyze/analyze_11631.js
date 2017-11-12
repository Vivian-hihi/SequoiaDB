/************************************
*@Description:  将未被修改的统计信息重新加载至缓存 
*@author:      liuxiaoxuan
*@createdate:  2017.11.11
*@testlinkCase: seqDB-11631
**************************************/
function main()
{	
   if (commIsStandalone(db))
   {
      println("skip standalone environment");
		return ;
   }
	
   var csName = COMMCSNAME + "11631";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
	//create cl	
	var clName = COMMCLNAME + "11631";
	var dbcl = commCreateCL( db, csName, clName );
	
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
	
	println("check result before analyze success!");
	
	//invoke analyze
	var options = { Mode : 3, Collection : csName + "." + clName };
   analyze( db, options );
	
	//check after analyze success
	checkStat( db, csName, clName, "a", false, false );
	
	//check the query explain after analyze
	var findConf = {a : 9000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain(actExplains, expExplains);	

   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain(actExplains, expExplains);
	
	println("check result after analyze success!");
	
	// modify SYSSTAT info 
	var groupName = commGetCLGroups( db, csName + "." + clName );
	var groups = commGetGroups( db, false, groupName[0] );
	
	var matcher = [{a: 0},{a: 1},{a:9000}];
	modifySYSInfo( groups, csName, clName, "a", matcher );
	
	// reload analyze again
	var options = { Mode : 4, Collection : csName + "." + clName };
   analyze( db, options );
	
	//check after reload analyze success
	checkStat( db, csName, clName, "a", true, true );
	
	//check the query explain after analyze
	var findConf = {a : 9000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain(actExplains, expExplains);
	
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains );
	
	println("check result after reload success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
}

function modifySYSInfo( groups, csName, clName, indexName, matcher )
{
	try
	{	
    	 //modify each node SYSSTAT info
       for(var j= 1; j < groups[0].length; j++)
       {
          var hostName = groups[0][j].HostName;
          var svcName = groups[0][j].svcname;
          var dataDB = new Sdb(hostName, svcName);
		    var rec = dataDB.SYSSTAT.SYSINDEXSTAT.find().toArray();
		
		    if(0 < rec.length)
		    {
			    dataDB.SYSSTAT.SYSINDEXSTAT.upsert({"$set": {"MCV": {"Values": matcher, "Frac": [500,500,9000]}}},
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
