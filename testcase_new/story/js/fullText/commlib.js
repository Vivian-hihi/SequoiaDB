/****************************************************
@description:	commlib of FullText		
@Date : 2018-09-18
@Author: liuxiaoxuan
                     
****************************************************/
var cmd = new Cmd();     
var HEADER = "'Content-Type: application/json'";
var HTTP = "'http://" + ESHOSTNAME + ":" + ESSVCNAME;

/******************************************************************************
*@Description : do some operations related to ES, such as:
                do queries by rest
                check index sync to ES
@usage:         var es = new ESOperator();
                var esRecords = es.findFromES( "esIndexName", querycond );
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
        * check records all sync to elasticsearch by comparing count and lid
        *****************************************************************/
   this.checkFullSyncToES = function (esIndexName, expectCount, cappedCL)
   {
      if(!isExistIndexInES(esIndexName))
      {
         throw buildException("isExistIndexInES()","index name isExist"," index name exsit", "exsit","not exsit");
      }

      this.checkCountInES(esIndexName, expectCount);
      this.checkLidInES(cappedCL, esIndexName);
   }
	
   /*****************************************************************
   * check records is full sync to elasticsearch by comparing count  
   *****************************************************************/
   this.checkCountInES = function (esIndexName, expectCount)
   {
      //the longest waiting time is 600S
      var isSync = false;
      var timeout = 600;
      var doTimes = 0;
      var interval = 1; //interval 1s
   
      while(true)
      {
         var actCount = this.countFromES(esIndexName);
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
   * check records is sync to elasticsearch by comparing lastLogicalID and SDBCOMMITID  
   *****************************************************************/
   this.checkLidInES = function (cappedCL, esIndexName)
   {
      //the longest waiting time is 600S
      var isSync = false;
      var timeout = 600;
      var doTimes = 0;
      var interval = 1; //interval 1s
   
      var md = new DBOperator();
      var lastLogicalID = md.getLastLID(cappedCL);
      while(true)
      {
         var commitID = this.getCommitIDFromES(esIndexName); 
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
   * check if index is exist in elasticsearch      
   *****************************************************************/
   function isExistIndexInES(esIndexName)
   {
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
   this.getCappedCL = function ( db, csName, clName, textIndexName )
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
   this.getESIndexName = function (db, csName, clName, textIndexName)
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
      var sortConf = {"_id" : 1};
      var records = this.findFromCL(cappedCL, null, null, sortConf, null);
      if(records.length > 0)
      { 
         lastLogicalID = records[records.length-1]["_id"];
      } 

      return lastLogicalID;
   }
	
   /*****************************************************************
   * find records by options
   *****************************************************************/
   this.findFromCL = function (dbcl, findConf, selectorConf, sortConf, hintConf, limitConf)
   {
      if ( typeof(selectorConf) == "undefined" ) { selectorConf = null; }
      if ( typeof(findConf) == "undefined" ) { findConf = null; }
      if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
      if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
	  if ( typeof(limitConf) == "undefined" ) { limitConf = null; }
  
      //find({"":{"$Text":{"query":{"match":{"a" : "test"}}}}}) 
      var rc = dbcl.find(findConf, selectorConf).sort(sortConf).hint(hintConf).limit(limitConf);
  
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
@description:   check records if equals between cl and es       
@input:         clRecords
                esRecords 
******************************************************************/
function checkRecords(clRecords, esRecords)
{
   if(clRecords.length !== esRecords.length)
   {
      throw buildException("checkRecords()", "check records", "check records length", clRecords.length, esRecords.length);
   }

   // match nothing, check success
   if(clRecords.length == 0)
   {
      println("check recoreds success!");
      return;
   }
	
   // if match something, get all keys in one object
   var keys = new Array();
   for(var key in clRecords[0])
   {
      keys.push(key);
   }
	
   // sort all keys of obj in clRecords and esRecords
   for(var i in keys)
   {
      clRecords.sort(sortObjectInArray(keys[i]));
      esRecords.sort(sortObjectInArray(keys[i]));
   }

   // compare array  
   for(var i = 0; i < clRecords.length; i++)
   {
      for(var j in keys)
      {
         var key = keys[j];
         if(clRecords[i][key] != esRecords[i][key])
         {
            throw buildException("checkRecords", "check record fail", "fail",
                        JSON.stringify(clRecords[i]), JSON.stringify(esRecords[i]));
         }	
      }
   }
	
   println("check recoreds success!");
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
