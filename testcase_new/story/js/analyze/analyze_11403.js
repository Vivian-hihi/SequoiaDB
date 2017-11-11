/************************************
*@Description: renameCS更新统计信息 (只支持standalone模式)
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11403
**************************************/
function main()
{
	if(!commIsStandalone(db))
	{
		println("not standalone enviroment");
		return;
	}
	
   var csName = COMMCSNAME + "11403";
	var csName_new = "newCsName"
	commDropCS( db, csName, true, "drop CS in the beginning" );
	commDropCS( db, csName_new, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
	
	//create CLs
	var clName = COMMCLNAME + "11403";
	var dbcl = commCreateCL( db, csName, clName );

	commCreateIndex( dbcl, "a", {a : 1}, false )

	var insertNums = 3000;
	var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
	insertSameDatas( dbcl, insertNums, sameValues );
	
	//check before invoke analyze
	checkStat( db, csName, clName, "a", false, false );
	
	//check the query explain before analyze
	var findConf = {a : 9000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   checkExplain( actExplains, expExplains )
	
	println("check result before analyze success!");
   
	//invoke analyze
	var options = {CollectionSpace: csName};
   analyze( db, options );
   
   //check after analyze
	checkStat( db, csName, clName, "a", true, true );
   
   //check the query explain after analyze
	var findConf = {a : 9000};
	var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   checkExplain( actExplains, expExplains )
	
   println("check result after analyze success!");
	
	//rename CS
	var oldCsName = csName;
	var newCsName = csName_new;
	renameCS(oldCsName, newCsName);
	
	//check newCL's anaylze info
	checkStat( db, newCsName, clName, "a", true, true );
	
	//check analyze result
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
	var newCL = db.getCS(newCsName).getCL(clName);
	var actExplains = getCommonExplain( newCL, findConf);
   checkExplain( actExplains, expExplains )
	
	println("check result after rename success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
	commDropCS( db, csName_new, true, "drop CS in the end" );
}

function renameCS( oldCsName, newCsName )
{
   try
   {
      db.renameCS( oldCsName, newCsName );
   }
	catch(e)
   {
      throw buildException("renameCS", e, "rename cs", "rename success", e);
   }
}
main();
