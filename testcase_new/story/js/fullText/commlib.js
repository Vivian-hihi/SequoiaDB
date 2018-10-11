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
   this.isExistIndexInES = function (esIndexName)
   {
      println("begin to check index name exists in ES");
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
         }
         catch(e)
         {
            throw buildException("isExistIndexInES()", "index name isExist", str, "success","fail");
         }
      
         if(!isExist)
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
            println("check index name sync to ES success!");
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
   this.getCappedCL = function ( csName, clName, textIndexName )
   {
      var clFullName = csName + "." + clName;
      var dbcl = db.getCS( csName ).getCL( clName );
      var cappedCLName = this.getCappedCLName( dbcl, textIndexName ); 
      var clGroups = commGetCLGroups( db, clFullName );
      var cappedCL = db.getRG( clGroups[0] ).getMaster().connect().getCS( cappedCLName ).getCL( cappedCLName );
      return cappedCL;
   }

   /*****************************************************************
   * get es index name, rule: 
   * cappedCLName: SYS_uniqueId_textIndexName  
   * esIndexName:  sys_uniqueId_textIndexName_clGroupName							  
   *****************************************************************/
   this.getESIndexName = function (csName, clName, textIndexName)
   {
      // check cappedcl name is valid
      var dbcl = db.getCS(csName).getCL(clName);
      var cappedCLName = this.getCappedCLName(dbcl, textIndexName);
	
      // get es index names
      var esIndexNames = new Array();
      var clGroupNames = commGetCLGroups(db, csName + "." + clName);
      for(var i in clGroupNames)
      {
         esIndexNames.push(cappedCLName.toLowerCase() + "_" + clGroupNames[i]);	
      }
	
      // if common cl, return one index
      if(esIndexNames.length == 1) { return esIndexNames[0]; }

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
   this.findFromCL = function (dbcl, findCond, selectorCond, sortCond, hintCond, limitCond)
   {
      if ( typeof(selectorCond) == "undefined" ) { selectorCond = null; }
      if ( typeof(findCond) == "undefined" ) { findCond = null; }
      if ( typeof(sortCond) == "undefined" ) { sortCond = null; }
      if ( typeof(hintCond) == "undefined" ) { hintCond = null; }
      if ( typeof(limitCond) == "undefined" ) { limitCond = null; }
  
      //find({"":{"$Text":{"query":{"match":{"a" : "test"}}}}}) 
      var rc = dbcl.find(findCond, selectorCond).sort(sortCond).hint(hintCond).limit(limitCond);
  
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
   var esIndexName = dbOpr.getESIndexName(csName, clName, textIndexName);
   var cappedCL = dbOpr.getCappedCL(csName, clName, textIndexName);
   if(!esOpr.isExistIndexInES(esIndexName))
   {
      throw buildException("isExistIndexInES()","index name isExist"," index name exsit", "exsit","not exsit");
   }

   checkCountInES(esIndexName, expectCount);
   checkLidInES(esIndexName, cappedCL);
}
	
/*****************************************************************
@description:   check records is full sync to elasticsearch by comparing count         
@input:         csName
                clName
                expectCount    
******************************************************************/
function checkCountInES(esIndexName, expectCount)
{
   println("begin to check count in ES");
   //the longest waiting time is 600S
   var isSync = false;
   var timeout = 600;
   var doTimes = 0;
   var interval = 1; //interval 1s
   
   while(true)
   {
      // remove the count of SDBCOMMITID
      var actCount = esOpr.countFromES(esIndexName) - 1;
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
function checkLidInES(esIndexName, cappedCL)
{
   println("begin to check commitID in ES");
   //the longest waiting time is 600S
   var isSync = false;
   var timeout = 600;
   var doTimes = 0;
   var interval = 1; //interval 1s
   
   var lastLogicalID = dbOpr.getLastLID(cappedCL);
   while(true)
   {
      var commitID = esOpr.getCommitIDFromES(esIndexName); 
      if(commitID == lastLogicalID)
      {
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
@description:   check result        
@input:         expectResult
                actResult
******************************************************************/
function checkResult(expectResult, actResult, sortOption)
{
   if(expectResult.length !== actResult.length)
   {
      throw buildException("checkResult()", "check records", "check records length", expectResult.length, actResult.length);
   }

   // match nothing, check success
   if(expectResult.length == 0)
   {
      println("check result success!");
      return;
   }
	
   // if match something, get all keys in one object
   var keys = new Array();
   for(var key in expectResult[0])
   {
      keys.push(key);
   }
	
   // sort all keys of obj in expectResult and actResult
   for(var i in keys)
   {
      expectResult.sort(sortObjectInArray(keys[i]));
      actResult.sort(sortObjectInArray(keys[i]));
   }

   // compare array  
   for(var i = 0; i < expectResult.length; i++)
   {
      for(var j in keys)
      {
         var key = keys[j];
         if(expectResult[i][key].toString() != actResult[i][key].toString())
         {
            throw buildException("checkResult", "check record fail", "fail",
                        JSON.stringify(expectResult[i]), JSON.stringify(actResult[i]));
         }	
      }
   }
	
   println("check results success!");
}

/*****************************************************************
@description:   sort object in array       
@input:         key
                x: object1's key
                y: object2's key					 
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
@description:   sort object in array       
@input:         key
                x: object1's key
                y: object2's key					 
******************************************************************/
function sortObjectInArray(key)
{
   return function (x, y)
   {
      if(x && y && typeof x === 'object' && typeof y === 'object')
      {
         if(x[key] === y[key])  {  return 0;  }	
         // value type equals
         if(typeof x[key] === typeof y[key])  {  return x[key] < y[key] ? -1: 1; }
         // value type not equals
         return typeof x[key] < typeof y[key]? -1: 1;   
      }    
      else
      {
         throw buildException("sortObjectInArray()", "sort object", "sortObj", "success", "fail key");                                       
      }	
      
   }
}

/******************************************************************************
*@Description : check consistency of all nodes
@input:         csName
                clName
                checkTimes
******************************************************************************/
function checkConsistency(csName, clName, checkTimes)
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
