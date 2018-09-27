/****************************************************
@description:	commlib of FullText		
@Date : 2018-09-18
@Author: liuxiaoxuan
                     
****************************************************/
//var ELASTICSEARCH_HOSTNAME = COORDHOSTNAME;
var ELASTICSEARCH_HOSTNAME = "192.168.31.57";
var ELASTICSEARCH_SVCNAME = 9200;
var cmd = new Cmd();     
var HEADER = "'Content-Type: application/json'";
var HTTP = "'http://" + ELASTICSEARCH_HOSTNAME + ":" + ELASTICSEARCH_SVCNAME;
var COND_QUERY_SDBCOMMITID = "{\"query\" : {\"match\" : {\"_id\": \"SDBCOMMIT\"}}}";

/*****************************************************************
@description:	run CURL command, to get return records from elasticsearch by rest	
@input:		elsticSearchIndexName 
            queryCond 

******************************************************************/
function findRecordsFromES(elasticSearchIndexName, queryCond){
   if(!isIndexFromESExist(elasticSearchIndexName)) 
   {
      var msg = elasticSearchIndexName + " is not exists in ES!";
      throw msg;
   }

   var records = new Array();
   // get curl command
   var str = "curl -H " + HEADER + " -XGET " + HTTP + "/" + elasticSearchIndexName + "/_search' -d '" + queryCond + "' 2>/dev/null";

   // to get info
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
	  println("findRecordsFromES:"+JSON.stringify(records));
   }
   catch(e)
   {
      println("Fail to run curl: " + str);
      throw e;
   }

   return records; 
}

/*****************************************************************
@description:   run CURL command, to get count from elasticsearch by rest      
@input:         elasticSearchIndexName 
                            run("curl -H "Content-Type: application/json" -XGET '192.168.31.42:9200/_count' -d '{ "query" : {"term" : {"_index" : "elasticsearch_index"}}}'"  2>/dev/null")

******************************************************************/
function getTotalCountFromES(elasticSearchIndexName)
{
   var count = 0;
   // get curl command
   var cond_query_count= "{\"query\" : {\"term\" : {\"_index\": \"" + elasticSearchIndexName  + "\"}}}";
   var str="curl -H " + HEADER + " -XGET " + HTTP + "/_count' -d '" + cond_query_count + "' 2>/dev/null";

   // get info
   try
   {
      var info = cmd.run(str);
      //get json
      var json = eval("(" + info + ")");
      count = json["count"];
   }
   catch(e)
   {
      println("Fail to run curl: " + str);
      throw e;
   }

   return count;
}

/*****************************************************************
@description:   run CURL command, to get SDBCOMMIT from elasticsearch by rest      
@input:         elasticSearchIndexName  

******************************************************************/
function getSdbCommitIDFromES(elasticSearchIndexName)
{ 
  if(!isIndexFromESExist(elasticSearchIndexName))
   {
       var msg = elasticSearchIndexName + " is not exists in ES!";
       throw msg;
   }

   var sdbCommitID = -1;
   // get curl command
   var str="curl -H " + HEADER + " -XGET " + HTTP + "/" + elasticSearchIndexName + "/_search' -d '" + COND_QUERY_SDBCOMMITID + "' 2>/dev/null";

   // to get info
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
      println("Fail to run curl: " + str);
      throw e;
   }

   return sdbCommitID; 
}

/*****************************************************************
@description:   get last lid       
@input:         dbcl

******************************************************************/
function getLastLogicalID(dbcl)
{
   var lastLogicalID = -1;
   var sortConf = {"_id" : -1};
   var records = findByOptions(dbcl, null, sortConf, null);
   if(records.length > 0)
   { 
      lastLogicalID = records[0]["_id"];
   } 

   return lastLogicalID;
}

/*****************************************************************
@description:   check all records is sync to elasticsearch       
@input:         elsticSearchIndexName 
                expectCount
******************************************************************/
function checkAllRecordsSyncToESByCount(elasticSearchIndexName, expectCount)
{
   if(!isIndexFromESExist(elasticSearchIndexName))
   {
       var msg = elasticSearchIndexName + " is not exists in ES!";
       throw msg;
   }
  
   //the longest waiting time is 600S
   var isSync = false;
   var timeout = 600;
   var doTimes = 0;
   
   while(true)
   {
      var actCount = getTotalCountFromES(elasticSearchIndexName);
      // if expect count < act count, exit
      if( actCount > expectCount ) 
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
            doTimes+=10;
            // interval 10s each time
            sleep(10000);
         }
         else
         {
            throw "check ES synchronization time out";
         }     
      }
      else 
      {
         println("check all sync to ES success!");
         break;
      }
   } 

}

/*****************************************************************
@description:   check all records is sync to elasticsearch       
@input:         dbcl
                elsticSearchIndexName 
******************************************************************/

function checkAllRecordsSyncToESByLid(dbcl, elasticSearchIndexName)
{
   if(!isIndexFromESExist(elasticSearchIndexName))
   {
       var msg = elasticSearchIndexName + " is not exists in ES!";
       throw msg;
   }

   //the longest waiting time is 600S
   var isSync = false;
   var timeout = 600;
   var doTimes = 0;
   
   var lastLogicalID = getLastLogicalID(dbcl);
   while(true)
   {
      var sdbCommitID = getSdbCommitIDFromES(elasticSearchIndexName); 
      if(sdbCommitID == lastLogicalID)
      {
         isSync = true;
      }
      
      if(!isSync)
      {
         if(doTimes < timeout)
         {
            doTimes+=10;
            // interval 10s each time
            sleep(10000);
         }
         else
         {
            throw "check ES synchronization time out";
         }
      }
      else
      {
         println("check all sync to ES success!");
         break;
      }
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
      println("clRecords length: " + clRecords.length + ", esRecords length: " + esRecords.length);
      throw "check length failed!";
   }

   for(var i = 0; i < clRecords.length; i++)
   {
      if(JSON.stringify(esRecords).indexOf(JSON.stringify(clRecords[i])) == -1)
      {
         throw buildException("checkRecords", "check record fail", "fail",
                                          JSON.stringify(clRecords[i]), JSON.stringify(esRecords));
      }
   }

   for(var j = 0; j < esRecords.length; j++)
   {
      if(JSON.stringify(clRecords).indexOf(JSON.stringify(esRecords[j])) == -1)
      {
         throw buildException("checkRecords", "check record fail", "fail",
                                          JSON.stringify(esRecords[j]), JSON.stringify(clRecords));
      }
   }

   println("check recoreds success!");
}

/*****************************************************************
@description:   check if index is exist in elasticsearch       
@input:         elasticSearchIndexName 
******************************************************************/
function isIndexFromESExist(elasticSearchIndexName)
{
   // get curl command
   var str="curl -H " + HEADER + " -XGET " + HTTP + "/" + elasticSearchIndexName + "' 2>/dev/null";
 
	//the longest waiting time is 60S
	var isExist = false;
   var timeout = 60;
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
         println("Fail to run curl: " + str);
         throw e;
      }
      
      if(!isExist)
      {
         if(doTimes < timeout)
         {
            doTimes+=5;
            // interval 5s each time
            sleep(5000);
         }
         else
         {
            throw "check ES Index synchronization time out";
         }
      }
      else
      {
         break;
      }
   }
 
   return isExist;
}

/*****************************************************************
@description:   find records by options
@input:         dbcl
                findConf
                sortConf 
                hintConf
                filterFields: ["a","b"]
******************************************************************/
function findByOptions(dbcl, findConf, sortConf, hintConf, filterFields)
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   if ( typeof(filterFields) == "undefined" ) { filterFields = new Array(); }
  
   //find({"":{"$Text":{"query":{"match":{"a" : "test"}}}}}) 
   var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf).toArray();
  
   //filter and delete keys
   var records = new Array(); 
   for(var i = 0; i < rc.length; i++) 
   {
      var record = eval("(" + rc[i] + ")");
      // delete filtered keys
      for(var key in record)
      {
         if(filterFields.length <= 0) { break; }
         if(filterFields.indexOf(key) == -1)
         {
            delete record[key];
         }
      }
      
      records.push(record);
   }
   return records;
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
   
/******************************************************************************
*@Description : get UniqueID
@input:         clFullName
******************************************************************************/  
function getUniqueID( clFullName )
{
	var cursor = db.snapshot( 8, { Name:clFullName } );
	var UniqueID = 0;
	while( cursor.next() )
	{
	    UniqueID = cursor.current().toObj()["UniqueID"];
	}
	return UniqueID;
}
