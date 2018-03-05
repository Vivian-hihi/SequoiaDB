/************************************
*@Description:capped cl, insert with _id and pop datas
*@author:      liuxiaoxuan
*@createdate:  2017.10.09
*@testlinkCase: seqDB-11774
**************************************/
function main()
{
   var csName = COMMCSNAME + "11774";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "11774";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
	//insert datas , _id locate in the ending
   var data = {a:1,_id:0};
	for(var i = 0; i < 3; i++)
   {
		insertDatas(dbcl,data);
	}
   
	//check find and count
	var expectResult = [{"_id": 0,"a": 1},
	                    {"_id": 56,"a": 1},
	                    {"_id": 112,"a": 1}];
	var sortConf = {_id:1};
   checkRecords( dbcl, null, null, sortConf, null, null, expectResult );
	
	var expectCount = 3;
   checkCount( dbcl, null, expectCount);
	
	//pop datas
	var logicalID = 0;
	var direction = -1;
	pop( dbcl, logicalID, direction );
   
	//insert datas,_id locate in the front/ending
	var doc = [];
	var insertNums = 100;
   for( var i = 0; i < insertNums; i++) 
	{ 
      doc.push({_id:i, a:i});
	   doc.push({a:i, _id:i});
	   doc.push({a:i});
	} 	
	insertDatas(dbcl , doc);
	
	//check count
	expectCount = 300;
   checkCount( dbcl, null, expectCount);
	//check find
	var expectIDs = [];
	for(var i = 0; i < expectCount; i++)
	{
		expectIDs.push(i * 56);
	}
	checkLogicalID( dbcl, null, null, sortConf, null, null, expectIDs);
	
	//drop cl and createCL again
	commDropCL( db, csName, clName,  'drop CL' );
	dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
	
	//insert again 
	insertDatas(dbcl,doc);
	
	//check count and find
	checkCount( dbcl, null, expectCount);
	checkLogicalID( dbcl, null, null, sortConf, null, null, expectIDs);
	
	//truncate datas 
	removeAllDatas(dbcl);
	
	//insert again
	insertDatas(dbcl,doc);
	
	//check count and find
   checkCount( dbcl, null, expectCount);
	checkLogicalID( dbcl, null, null, sortConf, null, null, expectIDs);
	
	//pop datas 
	pop( dbcl, logicalID, direction )
	
	//check after pop
	expectCount = 0;
	checkCount( dbcl, null, expectCount);
	
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();



