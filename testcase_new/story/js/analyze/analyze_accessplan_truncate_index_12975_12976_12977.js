/************************************
*@Description: 普通表上truncate、创建索引、删除索引，清空缓存功能验证  
*@author:      liuxiaoxuan
*@createdate:  2018.01.15
*@testlinkCase: seqDB-12975/12976/12977
**************************************/
function main()
{
   if(commIsStandalone(db))
   {
      println("skip standalone enviroment");
      return;
   }

   var csName = COMMCSNAME + "12975_12976_12977";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                                      	
   commCreateCS( db, csName, false, "" );
		
   //create cl
   var clName = COMMCLNAME + "12975_12976_12977";
   var dbcl = commCreateCL( db, csName, clName );                                                  
       
   //get and connect to primary/slave node
   var groupName = commGetCLGroups(db, csName + '.' + clName);                                                       	
   var group = db.getRG(groupName[0]);
   var priNode = group.getMaster().connect();
   var slaveNode = group.getSlave().connect();
  
   var dbclPrimary = priNode.getCS(csName).getCL(clName);
   var dbclSlave = slaveNode.getCS(csName).getCL(clName);

   println('master node: ' + priNode + ", slave node: " + slaveNode);
   println('dbcl primary: ' + dbclPrimary + ", dbcl slave: " + dbclSlave);
                                                  	
   //insert data
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );

   var options = {CollectionSpace: csName};
   analyze( db, options );

   //check after analyze 
   checkStat( db, csName, clName, "a", true, false );

   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                     
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );

   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                     
   //check snapshot access plans
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                   "AccessCount": 1}];
   var expectAccessPlanSlave = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                 "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //create index
   commCreateIndex( dbcl, "a", {a : 1}, false );
	
   //check snapshot access plans
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [];
   var expectAccessPlanSlave = [];

   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //same query 
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
                                     
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );

   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );             
                        
   //check snapshot access plans
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                   "AccessCount": 1}];
   var expectAccessPlanSlave = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                 "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //execute analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                        	
   //check after analyze 
   checkStat( db, csName, clName, "a", true, true );
   
   //check snapshot access plans after analyze before query
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [];
   var expectAccessPlanSlave = [];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}]; 
                                     
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );

   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );                        

   //check snapshot access plans after query
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                   "AccessCount": 1}];
   var expectAccessPlanSlave = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                 "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //execute truncate
   dbcl.truncate();
   
   //check analyze
   checkStat( db, csName, clName, "a", false, false );

   //check snapshot access plans after truncate before query
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [];
   var expectAccessPlanSlave = [];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );  

   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}]; 
                                     
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );

   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );     

   //check snapshot access plans after query
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                   "AccessCount": 1}];
   var expectAccessPlanSlave = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                 "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );          

   //insert data
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );

   //execute analzye
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                        	
   //check after analyze 
   checkStat( db, csName, clName, "a", true, true );

   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}]; 
                                     
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );

   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );                        

   //check snapshot access plans after query
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                   "AccessCount": 1}];
   var expectAccessPlanSlave = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                 "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //drop index
   dbcl.dropIndex('a');
   
   //check snapshot access plans after drop index
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [];
   var expectAccessPlanSlave = [];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );

   //same query
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}]; 
                                     
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );

   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );                        

   //check snapshot access plans after query
   var actAccessPlanPrimary = priNode.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var actAccessPlanSlave = slaveNode.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlanPrimary = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                   "AccessCount": 1}];
   var expectAccessPlanSlave = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                 "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlanPrimary, actAccessPlanPrimary );
   checkSnapShotAccessPlans( expectAccessPlanSlave, actAccessPlanSlave );
 
   commDropCS( db, csName, true, "drop CS in the end" );
}

main();
