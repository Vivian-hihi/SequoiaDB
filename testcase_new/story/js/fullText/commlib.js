/****************************************************
@description:	commlib of FullText		
@Date : 2018-09-18
@Author: liuxiaoxuan
                     
****************************************************/
var cmd = new Cmd();     
var HEADER = "'Content-Type: application/json'";
var HTTP = "'http://" + ESHOSTNAME + ":" + ESSVCNAME;
var esOpr = new ESOperator();
var dbOpr = new DBOperator();
var CREATEINDEXSYNCOPERATION = 0;
var DELETEINDEXSYNCOPERATION = 1;

// create WORKDIR in local host
commMakeDir( "localhost", WORKDIR );

/******************************************************************************
*@Description : do some operations related to ES, such as:
                do queries by rest
                check index sync to ES
@usage:         var es = new ESOperator();
                var esRecofindFromES( "esIndexName", querycond );
                es.checkFullSyncToES("esIndexName", expectCount, cappedCL)
******************************************************************************/
function ESOperator()
{
   /*****************************************************************
   * run CURL command, to get return records from elasticsearch by rest
   *****************************************************************/
   this.findFromES = function (esIndexName, queryCond)
   {
      var records = new Array();
      // get curl command
      var str = "curl -H " + HEADER + " -XGET " + HTTP + "/" + esIndexName 
                      + "/_search' -d '" + queryCond + "' 2>/dev/null";

      // to get records from ES
      try
      {
         var info = cmd.run(str); 
         //get json
         var json = eval("(" + info + ")");
         var array = json["hits"]["hits"];
         for(var i = 0; i < array.length; i++)
         {
            var _id = array[i]["_id"];
            if (_id == "SDBCOMMIT")  continue;
            var obj = array[i]["_source"];
            records.push(obj);
         }
      }
      catch(e)
      {
          throw buildException("findFromES()", "find from es", str, "success","fail");
      }

      return records; 
   }
	
   /*****************************************************************
   * run CURL command, to get count from elasticsearch by rest
   *****************************************************************/
   this.countFromES = function (esIndexName)
   {
      var count = 0;
      // get curl command
      var queryCount = "{\"query\" : {\"term\" : {\"_index\": \"" + esIndexName  + "\"}}}";         	            
      var str = "curl -H " + HEADER + " -XGET " + HTTP + "/_count' -d '" + queryCount 
	                  + "' 2>/dev/null";

      // get count from ES
      try
      {
         var info = cmd.run(str);
         //get json
         var json = eval("(" + info + ")");
         count = json["count"];
      }
      catch(e)
      {
         throw buildException("countFromES()", "count from es", str, "success","fail");
      }

      return count;
   }

   /*****************************************************************
   * run CURL command, to get SDBCOMMITID from elasticsearch by rest  
   *****************************************************************/
   this.getCommitIDFromES = function (esIndexName)
   { 
      var commitID = -1;
	
      var querySdbCommitID = "{\"query\" : {\"match\" : {\"_id\": \"SDBCOMMIT\"}}}";
      // get curl command
      var str="curl -H " + HEADER + " -XGET " + HTTP + "/" + esIndexName 
                      + "/_search' -d '" + querySdbCommitID + "' 2>/dev/null";

      // to get SDBCOMMITID from ES
      try
      {
         var info = cmd.run(str);
         //get json
         var json = eval("(" + info + ")");
         var array = json["hits"]["hits"];
         if(array.length == 1)
         {  
            commitID = array[0]["_source"]["_lid"];
         }
      }
      catch(e)
      {
         throw buildException("getCommitIDFromES()", "get commitid from es", str, "success","fail");
      }

      return commitID; 
   }

   /*****************************************************************
   * check if index is exist in elasticsearch      
   *****************************************************************/
   this.isExistIndexInES = function (esIndexName, syncOperation)
   {
      // CREATEINDEXSYNCOPERATION: judge index exist in ES (by default)
      // DELETEINDEXSYNCOPERATION: judge index not exist in ES
      if(typeof(syncOperation) == "undefined")  { var syncOperation = CREATEINDEXSYNCOPERATION; }
      if(syncOperation != CREATEINDEXSYNCOPERATION &&
             syncOperation !=  DELETEINDEXSYNCOPERATION)
      {
          println("syncOperation: " + syncOperation);
          throw buildException("isExistIndexInES()", "identify syncOperation", "identify syncOperation", syncOperation, syncOperation);
      }
      
      println("begin to check index name in ES");
      // get curl command
      var str = "curl -H " + HEADER + " -XGET " + HTTP + "/" + esIndexName + "' 2>/dev/null";
 
      //the longest waiting time is 300s
      var isExist = false;
      var timeout = 300;
      var doTimes = 0;
      var interval = 1; //interval 1s
   
      while(true)
      {
         try
         {
            var info = cmd.run(str);
            //get json
            var json = eval("(" + info + ")");
            var error = json["error"];
            if(typeof(error) == "undefined")  { isExist = true; }	//without error	
            else  { isExist = false; }	//with error
         }
         catch(e)
         {
            throw buildException("isExistIndexInES()", "check index name exist", str, "success","fail");
         }
         
         // when judgeType is CREATEINDEXSYNCOPERATION: check index name in ES exists
         // when judgeType is DELETEINDEXSYNCOPERATION: check index name in ES not exists
         var judgeFlag = (syncOperation == CREATEINDEXSYNCOPERATION)? !isExist : isExist;
         if(judgeFlag)
         {
            if(doTimes * interval < timeout)
            {
               doTimes+=1;
               // interval 1s each time
               sleep(1000);
            }
            else
            {
               throw "check ES Index name synchronization time out";
            }
         }
         else
         {
            println("check index name In ES success!");
            break;
         }
      }
 
      return isExist;
   }	
}

/******************************************************************************
*@Description : get db datas, such as:
                get clname
                get query results
@usage:         var db = new DBOperator();
                var cappedCLName = db.getCappedCLName( dbcl, "textIndexName" );
******************************************************************************/
function DBOperator()
{
   /*****************************************************************
   * get cappedcl name 
   *****************************************************************/
   this.getCappedCLName = function (dbcl, textIndexName)
   {
      var cappedCLName = "";
      var idx = dbcl.getIndex(textIndexName);
      cappedCLName = idx.toObj().ExtDataName;
      return cappedCLName;
   }
   
   
   /*****************************************************************
   * get cappedcl  
   *****************************************************************/
   this.getCappedCLs = function ( csName, clName, textIndexName )
   {
      var clFullName = csName + "." + clName;
      var dbcl = db.getCS( csName ).getCL( clName );
      var cappedCLName = this.getCappedCLName( dbcl, textIndexName ); 
      var clGroups = commGetCLGroups( db, clFullName );
      // sort groupname, in order to mapping esIndexNames <-> cappedCLs
	   clGroups = removeDuplicateItems(clGroups);
      clGroups.sort();
      // get each cappedCL from each group
      var cappedCLs = new Array();
      for (var i in clGroups)
      {
         var cappedCL = db.getRG(clGroups[i]).getMaster().connect().getCS(cappedCLName).getCL(cappedCLName);
         cappedCLs.push(cappedCL);
      }
      return cappedCLs;
   }

   /*****************************************************************
   * get es index name, rule: 
   * cappedCLName: SYS_uniqueId_textIndexName  
   * esIndexName:  sys_uniqueId_textIndexName_clGroupName							  
   *****************************************************************/
   this.getESIndexNames = function (csName, clName, textIndexName)
   {
      // check cappedcl name is valid
      var dbcl = db.getCS(csName).getCL(clName);
      var cappedCLName = this.getCappedCLName(dbcl, textIndexName);
	
      // get es index names
      var esIndexNames = new Array();
      var clGroupNames = commGetCLGroups(db, csName + "." + clName);
      clGroupNames = removeDuplicateItems(clGroupNames);
      // sort groupname, in order to mapping esIndexNames <-> cappedCLs
      clGroupNames.sort();
      for(var i in clGroupNames)
      {
         esIndexNames.push(cappedCLName.toLowerCase() + "_" + clGroupNames[i]);	
      }
	
      // if sharding cl, return all indices
      return esIndexNames;
   }
	
   /*****************************************************************
   * get last _id from cappedCL, in order to compare with ES's SDBCOMMITID 
   * and ensure that records are all sync to ES
   *****************************************************************/
   this.getLastLID = function (cappedCL)
   {
      var lastLogicalID = -1;
      var sortCond = {"_id" : 1};
      var records = this.findFromCL(cappedCL, null, null, sortCond, null);
      if(records.length > 0)
      { 
         lastLogicalID = records[records.length-1]["_id"];
      } 

      return lastLogicalID;
   }
	
   /*****************************************************************
   * find records by options
   *****************************************************************/
   this.findFromCL = function (dbcl, findCond, selectorCond, sortCond, hintCond, limitCond, skipCond)
   {
      if ( typeof(selectorCond) == "undefined" ) { selectorCond = null; }
      if ( typeof(findCond) == "undefined" ) { findCond = null; }
      if ( typeof(sortCond) == "undefined" ) { sortCond = null; }
      if ( typeof(hintCond) == "undefined" ) { hintCond = null; }
      if ( typeof(limitCond) == "undefined" ) { limitCond = null; }
      if ( typeof(skipCond) == "undefined" ) { skipCond = null; }
  
      //find({"":{"$Text":{"query":{"match":{"a" : "test"}}}}}) 
      var rc = dbcl.find(findCond, selectorCond).sort(sortCond).hint(hintCond).limit(limitCond).skip(skipCond);
  
      var records = new Array();
      //get all records
      while(rc.next())
      {
         var record = rc.current().toObj();
         records.push(record);		
      }

      return records;
   }
}

/*****************************************************************
@description:   check records all sync to elasticsearch by comparing count and lid       
@input:         csName
                clName
                expectCount
******************************************************************/
function checkFullSyncToES(csName, clName, textIndexName, expectCount)
{
   var esIndexNames = dbOpr.getESIndexNames(csName, clName, textIndexName);
   var cappedCLs = dbOpr.getCappedCLs(csName, clName, textIndexName);

   // check indexnames sync to ES
   for(var i in esIndexNames)
   {
      if(!esOpr.isExistIndexInES(esIndexNames[i]))
      {
         throw buildException("checkFullSyncToES","check index name exist"," index name exsit", "exsit","not exsit");
      }
   }
   
   // check all indices sync to ES
   checkCountInES(esIndexNames, expectCount);
   checkLidInES(esIndexNames, cappedCLs);
}
	
/*****************************************************************
@description:   check records is full sync to elasticsearch by comparing count         
@input:         csName
                clName
                expectCount    
******************************************************************/
function checkCountInES(esIndexNames, expectCount)
{
   println("begin to check count in ES");
   //the longest waiting time is 600S
   var isSync = false;
   var timeout = 600;
   var doTimes = 0;
   var interval = 1; //interval 1s
  
   while(true)
   {
      // clear count every time
      var actCount = 0;
      // Add counts of all indices 
      for(var i in esIndexNames)
      {
         actCount += (esOpr.countFromES(esIndexNames[i]) - 1);
      }
      // if expect count < act count, exit
      if(actCount == expectCount)
      { 
         isSync = true;
      }
     
      //if expect count > act count, wait to expect count = act count
      if(!isSync)
      {
         if(doTimes * interval < timeout)
         {
            doTimes+=1;
            // interval 1s each time
            sleep(1000);
         }
         else
         {
            throw buildException("checkCountInES()", "count sync to es", "check ES reords synchronization", "success","time out");
         }     
      }
      else 
      {
         println("check all counts sync to ES success!");
         break;
      }
   } 
}
	
/*****************************************************************
@description:   check records is sync to elasticsearch by comparing lastLogicalID and SDBCOMMITID          
@input:         csName
                clName 
******************************************************************/   
function checkLidInES(esIndexNames, cappedCLs)
{
   println("begin to check commitID in ES");
   
   // if esIndexNames not mapping to cappedCLs, fail
   if(esIndexNames.length !== cappedCLs.length)
   {
      throw buildException("checkLidInES()", "lid sync to es", "check ES records synchronization", "success","fail");
   }

   //the longest waiting time is 600S
   var isSync = false;
   var timeout = 600;
   var doTimes = 0;
   var interval = 1; //interval 1s
   
   // get all lids from all groups
   var lastLogicalIDs = new Array();
   for(var i in cappedCLs)  {   lastLogicalIDs.push(dbOpr.getLastLID(cappedCLs[i]));   }
   while(true)
   {
      // get all commitids from all esIndexNames 
      var commitIDs = new Array();
      for(var i in esIndexNames)  {   commitIDs.push(esOpr.getCommitIDFromES(esIndexNames[i]));  } 

      // check if all indices finish sync
      for(var i in esIndexNames)  
      {   
         if(commitIDs[i] !== lastLogicalIDs[i])
         {
            isSync = false;
            break;
         }
         isSync = true;
      }
      
      if(!isSync)
      {
         if(doTimes * interval < timeout)
         {
            doTimes+=1;
            // interval 1s each time
            sleep(1000);
         }
         else
         {
            throw buildException("checkLidInES()", "lid sync to es", "check ES records synchronization", "success","time out");
         }
      }
      else
      {
         println("check lid sync to ES success!");
         break;
      }
   }
}

/*****************************************************************
@description:   check index in elasticsearch not exist       
@input:         csName
                clName
                textIndexName
******************************************************************/
function checkIndexNotExistInES(csName, clName, esIndexNames)
{
   // check indexnames in ES not exist
   for(var i in esIndexNames)
   {
      if(esOpr.isExistIndexInES(esIndexNames[i], DELETEINDEXSYNCOPERATION))
      {
         throw buildException("checkIndexNotExistInES()","check index name exist"," index name exsit", "not exsit","exsit");
      }
   }
}

/*****************************************************************
@description:   check result        
@input:         expectResult
                actResult
******************************************************************/
function checkResult(expectResult, actResult)
{
   if(expectResult.length !== actResult.length)
   {
      throw buildException("checkResult()", "check records", "check records length", expectResult.length, actResult.length);
   }

   // compare array  
   for( var i in expectResult )
   {
      var actRec = actResult[i];
      var expRec = expectResult[i];
   	
      for ( var f in expRec )
      {
         if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) ) 
         {
            throw buildException("checkResult()", "check record fail", "fail",
                    JSON.stringify(JSON.stringify(expRec)), JSON.stringify(actRec));
         }
      }
   }
   
   for( var j in actResult )
   {
      var actRec = actResult[j];
      var expRec = expectResult[j];
   	
      for ( var f in actRec )
      {
         if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
         {
            throw buildException("checkResult()", "check record fail", "fail",
                    JSON.stringify(JSON.stringify(expRec)), JSON.stringify(actRec));
         }
      }
   }
	
   println("check results success!");
}

/*****************************************************************
@description:   sort object in array, rg:
                array.sort(compare("key1", compare("key2", compare("key3"))));       
@input:         key
                o: object1's key
                p: object2's key					 
******************************************************************/
function compare(name, minor) {
   return function (o, p) {
      var a, b;
      if (o && p && typeof o === 'object' && typeof p === 'object') {
         a = o[name];
         b = p[name];
         if (a === b) {
            return typeof minor === 'function' ? minor(o, p) : 0;
         }
         if (typeof a === typeof b) {
            return a < b ? -1 : 1;
         }
         return typeof a < typeof b ? -1 : 1;
      } else {
         throw("error");
      }
   }
}

/*****************************************************************
*@Description : check consistency: LSNs consistency of all nodes, ES consistency
@input:         csName
                clName
******************************************************************/
function checkConsistency(csName, clName)
{
   // check LSN consistency
   if(csName == null) { var csName = "UNDEFINED"; }
   if(clName == null) { var clName = "UNDEFINED"; }

   var groups = commGetCLGroups( db, csName + "." + clName );
   
   //the longest waiting time is 600S
   var lsnFlag = false;
   var timeout = 600;
   var doTimes = 0; 
   
   //get primary nodes
   var primaryNodeLSNs = getPrimaryNodeLSNs(groups);
   while(true)
   {
      lsnFlag = checkLSN(groups, primaryNodeLSNs);
      if(!lsnFlag)
      {
         if(doTimes < timeout)
         {
            ++doTimes;
            sleep(1000);
         }
         else
         {
            throw "check lsn time out";
         }     
      }
      else 
      {
         break;
      }
   }

   println("check consistency success!");
}

/*****************************************************************
*@Description: check lsn consistency
@input:         groups
                primaryNodeLSNs
******************************************************************/
function checkLSN(groups, primaryNodeLSNs)
{
   var slaveNodeLSNs = getSlaveNodeLSNs(groups);
 
   var checkLSN = true;
   //比较主备节点lsn
   for(var i = 0; i < slaveNodeLSNs.length; ++i)
   {
      for(var j = 0; j < slaveNodeLSNs[i].length; ++j)
      {
         if(primaryNodeLSNs[i][0] > slaveNodeLSNs[i][j])
         {
            checkLSN = false;
            return checkLSN;
         }
      }
   }
   
   return checkLSN;
}

/*****************************************************************
*@Description: get lsn of primary node
@input:        groups
******************************************************************/
function getPrimaryNodeLSNs(groups)
{
   
   var datas = getNodesInGroups(groups);
   
   var LSNs = new Array();
   for(var i = 0; i < datas.length; ++i)
   {
      var nodesInGroup = datas[i];
      LSNs[i] = Array();
      for(var j = 0; j < nodesInGroup.length; ++j)
      { 
         var getSnapshot6 = eval( "(" + nodesInGroup[j].snapshot(6).toArray()[0] + ")" );
         
         var completeLSN = getSnapshot6.CompleteLSN;
         var isPrimary = getSnapshot6.IsPrimary;
         if(isPrimary)
         {
            LSNs[i][0] = completeLSN;
            break;
         }   
      }
   }
   
   return LSNs;
}

/*****************************************************************
*@Description: get lsn of slave node
@input:        groups
******************************************************************/
function getSlaveNodeLSNs(groups)
{
   var datas = getNodesInGroups(groups);
   
   var LSNs = new Array();
   for(var i = 0; i < datas.length; ++i)
   {
      var nodesInGroup = datas[i];
      LSNs[i] = Array();
      var f = 0;
      for(var j = 0; j < nodesInGroup.length; ++j)
      { 
         var getSnapshot6 = eval( "(" + nodesInGroup[j].snapshot(6).toArray()[0] + ")" );
         
         var completeLSN = getSnapshot6.CompleteLSN;
         var isPrimary = getSnapshot6.IsPrimary;
         if(!isPrimary)
         {
            LSNs[i][f++] = completeLSN;
         }   
      }
   }
   
   return LSNs;
}

/*****************************************************************
*@Description: get all nodes of all groups
@input:        groups
******************************************************************/
function getNodesInGroups(groups)
{
   var datas = new Array();
   
   //standalone
   if(true === commIsStandalone(db))
   {
      datas[0] = Array();
      datas[0][0] = db;
   }else{
      for (var i = 0 ; i < groups.length; ++i)
      {
         datas[i] = Array();
         
         var rg = db.getRG(groups[i]);
         var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")");
         var nodesInGroup = rgDetail.Group;
         for(var j = 0; j < nodesInGroup.length; ++j)
         {
            var hostName = nodesInGroup[j].HostName;
            var serviceName = nodesInGroup[j].Service[0].Name;
            datas[i][j] = new Sdb(hostName, serviceName);                                                                                                                                 
         }
         
      }
   }
   return datas;
}

/******************************************************************************
*@Description : check inpect result
@input:         csName
                clName
                checkTimes
******************************************************************************/
function checkInspectResult(csName, clName, checkTimes)
{
   if ( typeof(checkTimes) == "undefined" )  {  checkTimes = 5;  }

   var inspectBinFile = WORKDIR + "/" + "inspect_" + csName + "_" + clName + ".bin" ;
   var inspectReportFile = WORKDIR + "/" + "inspect_" + csName + "_" + clName + ".bin.report" ;
   var installPath = commGetInstallPath();   
   var inspectCommand = installPath + "/bin/sdbinspect" + " -d " + COORDHOSTNAME + ":" + COORDSVCNAME + " -c " + csName + " -l " + clName + " -o " + inspectBinFile + " -t " + checkTimes; 
   try 
   {  
      // exec sdbinspect 
      cmd.run(inspectCommand) ;
      var info = cmd.run("tail -n 1 " + inspectReportFile);
      var actResult = info.split("\n")[0].split("\:")[1].trim();
      var expectRusult = "exit with no records different";
      // compare result
      if(actResult == expectRusult)
      {
         println("check consistency success!") ;
      }
      else
      {
         println("check consistency fail, cl name: " + csName + "." + clName); 
      }
      // remove report files
      cmd.run("rm -f " + inspectBinFile);
      cmd.run("rm -f " + inspectReportFile);
   }   
   catch(e) 
   { 
      throw buildException("checkConsistency", "check consistency fail", "fail",
                                          e, e);  
   }   
}

/******************************************************************************
*@Description : insert data,skip the error -321
@input:         dbcl
                records
******************************************************************************/
function insertRecords(dbcl, records)
{
   try
   {
      dbcl.insert(records);
   }catch(e)
   {
      if(e !== -321)
      {
         throw buildException("insertRecords", "insert records fail", "fail", e, e);  
      }
   }
}

/******************************************************************************
*@Description : remove duplicate items in array
@input:         array
******************************************************************************/
function removeDuplicateItems(array)
{
   var uniqueArray = new Array();
   for (var i in array){
      if (uniqueArray.indexOf(array[i]) == -1){
         uniqueArray.push(array[i]);
      }
   }
   return uniqueArray;
}
