/************************************
*@Description:  指定id索引收集统计信息 
*@author:      liuxiaoxuan
*@createdate:  2017.11.10
*@testlinkCase: seqDB-11617
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11617";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
	var csOption = { PageSize: 4096 };
   commCreateCS( db, csName, false, "", csOption );
		
	//create cl	
	var clName = COMMCLNAME + "11617";
	var dbcl = commCreateCL( db, csName, clName );
	
	//insert
	var insertNums = 5000;
   insertDatas( dbcl, insertNums );
	
	//check before invoke analyze
	checkStat( db, csName, clName, "$id", false, false );
	
	//check the query explain of master/slave nodes 
	var findConf = {_id : 4000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( actExplains, expExplains );
	
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");
	
	//invoke analyze
	var options = {Collection: csName + "." + clName, Index: "$id"};
	analyze( db, options );
	
   //check after analyze
	checkStat( db, csName, clName, "$id", true, true );
   
   //check the query explain of master/slave nodes 
	var findConf = {_id : 4000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
	
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( actExplains, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains );

   println("check result after analyze success!");
}

function insertDatas( dbcl, insertNum )
{  
   try
   {
      var doc = [];
      for(var i = 0;i < insertNum;i++)
      {
         doc.push({_id:i, a:"test" + i});
      }
      dbcl.insert(doc);
   }
   catch(e)
   {
      throw buildException("insert datas", e, "insert", "insert success", e);
   }
}

main();
