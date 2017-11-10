/************************************
*@Description: renameCl更新统计信息(只支持standalone模式)
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11404
**************************************/
function main()
{
	if(!commIsStandalone(db))
	{
		println("not standalone enviroment");
		return;
	}
	
   var csName = COMMCSNAME + "11404";
	commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
	
	//create CLs
	var clName = COMMCLNAME + "11404";
	var dbcl = commCreateCL( db, csName, clName );

	commCreateIndex( dbcl, "a", {a : 1}, false )

   var insertNums = 3000;
   insertDatas( dbcl, insertNums );
	
	//check before invoke analyze
	checkStat( db, csName, clName, "a", false, false );
	
	//check the query explain before analyze
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
	
	println("check result before analyze success!");
   
	//invoke analyze
	var options = {Collection: csName + "." + clName};
   analyze( db, options );
   
   //check after analyze
	checkStat( db, csName, clName, "a", true, true );
   
   //check the query explain after analyze
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
	
   println("check result after analyze success!");
	
	//rename CL
	var oldClName = clName;
	var newClName = "newClName";
	renameCL(csName, oldClName, newClName);
	
	//check newCL's anaylze info
	checkStat( db, csName, newClName, "a", true, true );
	
	//check analyze result
	var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   checkExplain( db, csName, newClName, findConf, null, null, expExplains )
	
	println("check result after rename success!");
	
	commDropCS( db, csName, true, "drop CS in the end" );
}

function renameCL( csName, oldClName, newClName )
{
   try
   {
		var cs = db.getCS(csName);
      cs.renameCL( oldClName, newClName );
   }
	catch(e)
   {
      throw buildException("renameCL", e, "rename cl", "rename success", e);
   }
}
main();
