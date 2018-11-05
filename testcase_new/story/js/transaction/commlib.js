/* *****************************************************************************
@Description: sdb transaction common function 
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */

var cmd = new Cmd();     
var HEADER = "'Content-Type: application/json'";
var HTTP = "'http://" + ESHOSTNAME + ":" + ESSVCNAME;
var esOpr = new ESOperator();
var dbOpr = new DBOperator();

function dbNew( db )
{
   try
   {
      db = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
   }
   catch( e )
   {
      println( " new  Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbClose( db )
{
   try
   {
      db.close() ;
   }
   catch( e )
   {
      println( " close Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbArrayNew( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i] = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
      }
   }
   catch( e )
   {
      println( " new the " + i + "st Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbArrayClose( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i].close() ;
      }
   }
   catch( e )
   {
      println( " close the" + i + "st Sdb failed : " + e ) ;
      throw e ;
   }
}

/*******************************************************************************
@Description : 比较查询返回的结果（游标）与预期结果(数组)是否一致
@Modify list : 2018-10-15 zhaoyu init
*******************************************************************************/
function checkRec( rc, expRecs )
{				
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   
   //check count
	if( actRecs.length !== expRecs.length )
   {
   	//println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));
   	throw buildException("check count", null, "",
									expRecs.length, actRecs.length);
   }
   
   //check every records every fields
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual recs in cl= "+JSON.stringify(actRec)+"\n\nexpect recs= "+JSON.stringify(expRec));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
   
   //check every records every fields,actRecs as compare source
   for( var i in actRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in actRec )
   	{
   	   if(f == "_id")
   	   {
   	      continue;
   	   }
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec)); 		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
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

/******************************************************************************
*@Description : get db datas, such as:
                get clname
                get query results
@usage:         var db = new DBOperator();
                var cappedCLName = db.getCappedCLName( dbcl, "textIndexName" );
******************************************************************************/
function DBOperator()
{
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
   
   println("expResult : " + JSON.stringify(expectResult));
   println("actResult : " + JSON.stringify(actResult));

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