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
******************************************************************************/
function commElasticSearch()
{
   /*****************************************************************
   * run CURL command, to get return records from elasticsearch by rest
   *****************************************************************/
   this.findFromES = function (esIndexName, queryCond)
   {
      if(!isExistIndexInES(esIndexName)) 
      {
         throw buildException("isExistIndexInES()",e," index name exsit", "exsit","not exsit");
      }

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
          throw buildException("findFromES()", e, str, "success","fail");
      }

      return records; 
   }
	
   /*****************************************************************
   * run CURL command, to get count from elasticsearch by rest
   *****************************************************************/
   this.getTotalCountFromES = function (esIndexName)
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
         throw buildException("getTotalCountFromES()", e, str, "success","fail");
      }

      return count;
   }

   /*****************************************************************
   * run CURL command, to get SDBCOMMITID from elasticsearch by rest  
   *****************************************************************/
   this.getSdbCommitIDFromES = function getSdbCommitIDFromES(esIndexName)
   { 
      var sdbCommitID = -1;
	
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
            sdbCommitID = array[0]["_source"]["_lid"];
         }
      }
      catch(e)
      {
         throw buildException("getSdbCommitIDFromES()", e, str, "success","fail");
      }

      return sdbCommitID; 
   }

   /*****************************************************************
        * check records all sync to elasticsearch by comparing count and lid
        *****************************************************************/
   this.checkFullSyncToES = function (esIndexName, expectCount, cappedCL)
   {
      if(!isExistIndexInES(esIndexName))
      {
         throw buildException("isExistIndexInES()",e," index name exsit", "exsit","not exsit");
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
   
      while(true)
      {
         var actCount = this.getTotalCountFromES(esIndexName);
         // if expect count < act count, exit
         if(actCount > expectCount) 
         {
            println("check sync to ES fail: actCount: " + actCount + ", expectCount: " + expectCount);
            break;
         }
         else if(actCount == expectCount)
         { 
            isSync = true;
         }
      
         //if expect count > act count, wait to expect count = act count
         if(!isSync)
         {
            if(doTimes < timeout)
            {
               doTimes+=1;
               // interval 3s each time
               sleep(1000);
            }
            else
            {
               throw buildException("checkFullSyncToES()", e, "check ES reords synchronization", "success","time out");
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
   
      var md = new commGetMetaData();
      var lastLogicalID = md.getLastLogicalID(cappedCL);
      while(true)
      {
         var sdbCommitID = this.getSdbCommitIDFromES(esIndexName); 
         if(sdbCommitID == lastLogicalID)
         {
            isSync = true;
         }
      
         if(!isSync)
         {
            if(doTimes < timeout)
            {
               doTimes+=1;
               // interval 3s each time
               sleep(1000);
            }
            else
            {
               throw buildException("checkFullSyncToES()", e, "check ES records synchronization", "success","time out");
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
            throw buildException("isExistIndexInES()", e, str, "success","fail");
         }
      
         if(!isExist)
         {
            if(doTimes < timeout)
            {
               doTimes+=1;
               // interval 3s each time
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
*@Description : get some metadata, such as:
                get clname
					 get query results
******************************************************************************/
function commGetMetaData()
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
   * get es index name, rule: 
   * cappedCLName: SYS_uniqueId_textIndexName  
   *esIndexName:  sys_uniqueId_textIndexName_clGroupName							  
   *****************************************************************/
   this.getESIndexName = function (db, clFullName, textIndexName)
   {
      // check cappedcl name is valid
      var commCSName = clFullName.split(".")[0];
      var commCLName = clFullName.split(".")[1];
      var dbcl = db.getCS(commCSName).getCL(commCLName);
      var cappedCLName = this.getCappedCLName(dbcl, textIndexName);
	
      // get es index names
      var esIndexNames = new Array();
      var clGroupNames = commGetCLGroups(db, clFullName);
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
   this.getLastLogicalID = function (cappedCL)
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
   this.findFromCL = function (dbcl, findConf, selectorConf, sortConf, hintConf)
   {
      if ( typeof(selectorConf) == "undefined" ) { selectorConf = null; }
      if ( typeof(findConf) == "undefined" ) { findConf = null; }
      if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
      if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
  
      //find({"":{"$Text":{"query":{"match":{"a" : "test"}}}}}) 
      var rc = dbcl.find(findConf, selectorConf).sort(sortConf).hint(hintConf);
  
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

/******************************************************************************
*@Description : check results, such as:
                check records
					 check consistency
******************************************************************************/
function commCheckResult()
{
   /*****************************************************************
   * check records if equals between cl and es  
   *****************************************************************/
   this.checkRecords = function (clRecords, esRecords)
   {
      if(clRecords.length !== esRecords.length)
      {
          throw buildException("checkRecords()", e, "check records length", clRecords.length, esRecords.length);
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
	
      // sort all keys between objs in clRecords and esRecords
      for(var key in keys)
      {
         clRecords.sort(sortObjectInArray(key));
         esRecords.sort(sortObjectInArray(key));
      }
	
      // compare array  
      for(var i = 0; i < clRecords.length; i++)
      {
         for(var key in keys)
         {
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
   * sort object in array
   * x: object1, y: object2
   *****************************************************************/
   function sortObjectInArray(key)
   {
      return function (x, y)
      {
         if(x && y && typeof x === 'object' && typeof x === 'object')
         {
            if(x[key] === y[key])  {  return 0;  }	
            // value type equals
            if(typeof x[key] === typeof y[key])  {  return x[key] - y[key]; }
            // value type not equals
            return typeof a - typeof b;   
          }    
         else
         {
            throw buildException("sortObjectInArray()", e, "sortObj", "success", "fail key");                                       
         }	
     
      }
   }
	
   /*****************************************************************
   * check consistency of all nodes
   *****************************************************************/
   this.checkConsistency = function (csName, clName, checkTimes)
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
}
