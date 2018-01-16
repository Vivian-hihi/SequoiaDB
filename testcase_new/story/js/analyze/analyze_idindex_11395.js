/************************************
*@Description:  创建Id索引更新统计信息 、清空缓存 
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11395
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11395";
   commDropCS( db, csName, true, "drop CS in the beginning" );
	
   var csOption = { PageSize: 4096 };
   commCreateCS( db, csName, false, "", csOption );
		
   //create cl	
   var clOption = { AutoIndexId : false };
   var clName = COMMCLNAME + "11395";
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
	
   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave = db1.getCS(csName).getCL(clName);
	
   //insert
   var insertNums = 5000;
   insertDatas( dbcl, insertNums );
	                                                                 
   //invoke analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                                                       
   //check after analyze
   checkStat( db, csName, clName, "$id", true, false );

   //check the query explain of master/slave nodes 
   var findConf = {_id : 4000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:1}];
	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //query no explain
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
                                                                           
   var expectAccessPlansCluster = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];  
   if(commIsStandalone(db)) 
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   } 

   println("check result after create id index!");
   
   //create id index
   createIdIndex( dbcl );
 
   checkStat( db, csName, clName, "$id", true, false );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var expectAccessPlans = [];	
   checkSnapShotAccessPlans( expectAccessPlans, actAccessPlans );                                                                        

   //check the query explain of master/slave nodes 
   var findConf = {_id : 4000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //query no explain
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
                                                                           
   var expectAccessPlansCluster = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];  
   if(commIsStandalone(db)) 
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   } 

   println("check result after create id index!");

   //analyze after create id index
   var options = {CollectionSpace: csName};
   analyze( db, options );
   
   checkStat( db, csName, clName, "$id", true, true );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var expectAccessPlans = [];	
   checkSnapShotAccessPlans( expectAccessPlans, actAccessPlans );  

   //check the query explain of master/slave nodes 
   var findConf = {_id : 4000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //query no explain
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
                                                                           
   var expectAccessPlansCluster = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];  
   if(commIsStandalone(db)) 
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   } 

   println("check result after analyze success!");

   //drop id index
   dropIdIndex( dbcl );

   //check analyze result
   checkStat( db, csName, clName, "$id", true, false );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();
   var expectAccessPlans = [];	
   checkSnapShotAccessPlans( expectAccessPlans, actAccessPlans );  

   //check the query explain of master/slave nodes 
   var findConf = {_id : 4000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:1}];
	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   //query no explain
   querySameWithOutExplain( dbclPrimary, findConf );
   querySameWithOutExplain( dbclSlave, findConf );

   //check access plans
   var actAccessPlans = db.snapshot(11, {Collection : csName + "." + clName}).toArray();

   var expectAccessPlansStandAlone = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                       "AccessCount": 2} ];	
                                                                           
   var expectAccessPlansCluster = [{"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1},
                                   {"Query": {"$and": [{"_id": {"$et": {"$param": 0,"$ctype": 10}}}]},
                                    "AccessCount": 1} ];  
   if(commIsStandalone(db)) 
   {
      checkSnapShotAccessPlans( expectAccessPlansStandAlone, actAccessPlans );
   }
   else
   {
      checkSnapShotAccessPlans( expectAccessPlansCluster, actAccessPlans );
   } 

   println("check result after drop id success!");

   db1.close();
   commDropCS( db, csName, true, "drop CS in the end" );
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
function createIdIndex( dbcl )
{
   try
   {
      dbcl.createIdIndex();
   }
   catch(e)
   {
      throw buildException("create id index", e, "create IdIndex", "success", e);
   }
}
function dropIdIndex( dbcl )
{
   try
   {
      dbcl.dropIdIndex();
   }
   catch(e)
   {
      throw buildException("create id index", e, "create IdIndex", "success", e);
   }
}
function querySameWithOutExplain(dbcl, findConf, sortConf, hintConf)
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   
   //执行查询
   var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf);
   while(rc.next())
   {
   }
}
main();
