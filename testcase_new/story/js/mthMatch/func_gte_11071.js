/************************************
*@Description: find操作，匹配符$gte，匹配已存在的数组
*@author:      wangkexin
*@createdate:  2019.3.5
*@testlinkCase: 
**************************************/
function main()
{
	var csName = CHANGEDPREFIX + "_11072_CS";
	var clName = CHANGEDPREFIX + "_11072_CL";
	
	commDropCS(db, csName, true, "drop cs in the begin");
	var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );
	
	//insert data 
	cl.insert({a:[1,2,3]});
	
	var findCondition = {a:{$gte:[1,2,3]}};
	var expRecs = [{"a":[1,2,3]}];
	checkResult( cl, findCondition, null, expRecs, {_id:1} );
}
main()