/************************************
*@Description: random data,random matches,find data
*@author:      zhaoyu
*@createdate:  2016.11.3
*@testlinkCase:seqDB-9246 
**************************************/
//CLName
var indexScan_MainTable   = COMMCLNAME + '_main_indexScan';
var indexScan_subCL_Name1 = CHANGEDPREFIX + "_indexScan_subcl92461";
var indexScan_subCL_Name2 = CHANGEDPREFIX + "_indexScan_subcl92462";
var indexScan_subCL_Name3 = CHANGEDPREFIX + "_indexScan_subcl92463";

//query field name
var fieldNames = ['fieldName1','fieldName2','fieldName3','fieldName4','fieldName5','fieldName6'];

function main()
{
   //var startTime = new Date();
   //println("startTime:" + startTime.toLocaleString());
   //clean environment before test
   clearEnviron();
   
   //create main-sub cl for index scan
   var dbcl_IndexScan = createCLForIndexScan();
   
   //insert random numberical data
   insertRandomData( dbcl_IndexScan );
   println("insert data success");
      
   //create index for index scan
   createIndex( dbcl_IndexScan );
   println("createIndex success");
   
   //query use random conditon and check result
   var loopNum = 10;
   queryDataAndCheck( dbcl_IndexScan, loopNum );
   println("check result success");
   //var endTime = new Date();
   //println("endTime:" + endTime.toLocaleString());
}
main()
 
 /************************************
*@Description: unset the order for arr elements.
*@author:      zhaoyu 
*@createDate:  2016/7/19
*@parameters:               
**************************************/
function getRdmDataFromArr(arr)
{
   //unset the arr order
   var newArr = arr.sort(function(){return Math.random()-0.5});
   
   //convert array to object
   var obj = {};
   for(var i in newArr)
   {
      for (var k in newArr[i])
      {
         obj[k] = newArr[i][k];
      } 
   }
   return obj;
}

function arrToObj( arr )
{
   var obj = {};
   for(var i in arr)
   {
      for (var k in arr[i])
      {
         obj[k] = arr[i][k];
      } 
   }
   return obj;
}

//clean environment
function clearEnviron()
{
   commDropCL( db, COMMCSNAME, indexScan_subCL_Name1, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, indexScan_subCL_Name2, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, indexScan_subCL_Name3, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, COMMCSNAME, indexScan_MainTable, true, true,"drop CL in the beginning" ) ;
}

//create main-sub cl for index scan
function createCLForIndexScan()
{
   var indexScan_MainCLOption = {ShardingKey:{"No":1},ShardingType:"range", IsMainCL:true};
   var dbcl_IndexScan = commCreateCLByOption( db, COMMCSNAME, indexScan_MainTable, indexScan_MainCLOption, true, true );
   
   //create subcl
   var indexScan_subClOption1 = {ShardingKey:{"No":1},ShardingType:"range", ReplSize:0,AutoIndexId:false};
   commCreateCLByOption( db, COMMCSNAME, indexScan_subCL_Name1, indexScan_subClOption1, true, true );
   
   var indexScan_subClOption2 = {ShardingKey:{"No":1},ShardingType:"hash", ReplSize:0,AutoIndexId:false};
   commCreateCLByOption( db, COMMCSNAME, indexScan_subCL_Name2, indexScan_subClOption2, true, true );
   
   var indexScan_subClOption3 = {AutoIndexId:false};
   commCreateCLByOption( db, COMMCSNAME, indexScan_subCL_Name3, indexScan_subClOption3, true, true );
   
   //split cl
   indexScan_startCondition1 = {No:5000};
   splitGrInfo = ClSplitOneTimes( COMMCSNAME, indexScan_subCL_Name1, indexScan_startCondition1, null ); 
   
   indexScan_startCondition2 = {Partition:2014};
   splitGrInfo = ClSplitOneTimes( COMMCSNAME, indexScan_subCL_Name2, indexScan_startCondition2, null ); 
   
   //attach cl
   attachCL( dbcl_IndexScan, COMMCSNAME + "." + indexScan_subCL_Name1, { LowBound:{No:0},UpBound:{No:2222} } );
   attachCL( dbcl_IndexScan, COMMCSNAME + "." + indexScan_subCL_Name2, { LowBound:{No:2222},UpBound:{No:5555} } );
   attachCL( dbcl_IndexScan, COMMCSNAME + "." + indexScan_subCL_Name3, { LowBound:{No:5555},UpBound:{No:50000} } );
   
   return dbcl_IndexScan;
}

//create index
function createIndex( dbcl_IndexScan )
{
   for(var i = 0;i < fieldNames.length;i++)
   {
      var indexDef = {};
      var key = fieldNames[i];
      if(i%2 == 0)
      {
         indexDef[key] = 1;
      }
      else{
         indexDef[key] = -1;
      }
      var idxName = "fieldName" + i + "Index";
      
      try
      {
         dbcl_IndexScan.createIndex( idxName, indexDef, false, false, 1536 ) ;
      }
      catch( e )
      {
         buildException("createIndex", e, 
                        'createIndex( ' + idxName + ', ' + indexDef + ', false, false, 1536 )',
                        0, e);
      }
   }
}


//insert random numberical data
function insertRandomData( dbcl_IndexScan )
{
   for(var i= 0; i< 40;i++)
   {
      var rd = new dataGenerator();
      var recs = rd.getRecords( 50000, ["int", "long", "float", "array"], fieldNames );
      insertData(dbcl_IndexScan, recs);
      rd = null;
      recs.length = 0;  // release space
   }
}

//generate query condition and check result
function queryDataAndCheck( dbcl_IndexScan, loopNum )
{
   for(var i= 0; i< loopNum;i++)
   {
	   //set all matches support index scan
	   matches1 = [ "$et", "$gt", "$gte", "$lt", "$lte", "$ne" ];
	   matches2 = [ "$in", "$all" ];
	   
	   var findCondition1 = getFindCondition( 1, ["int", "long", "float" ], matches1 );
	   var findValue2 = getRandomArray();
	   var findCondition2 = getFindCondition( 1, "array", matches2 );
	   
	   findCondition3 = [ {$exists:1}, {$isnull:0} ];
	   
	   //convert array to object
	   findConditionObj1 = arrToObj( findCondition1 );
	   findConditionObj2 = arrToObj( findCondition2 );
	   findConditionObj3 = arrToObj( findCondition3 );
	   
	   var obj1 = mergeObj(findConditionObj1, findConditionObj2);
	   var obj2 = mergeObj(obj1, findConditionObj3);
	   
	   var findConditions = [];
	   var cnt = 0;
	   for( var j in obj2 )
	   { 
		  var subcond = {};
		  subcond[j] = obj2[j]
		  findConditions.push(subcond);
	   }
	   
	   //generate random find condition
	   var randomCondition = genRandomFindCondition( findConditions, fieldNames );
	   var randomConditionObj = {$and:randomCondition};
	   println("randomCondition:"+ JSON.stringify(randomConditionObj));
		
	   //get index scan result
	   var ixScanCursor = dbcl_IndexScan.find( randomConditionObj, null ).sort( { _id: 1 } );
	   
	   //get index scan explain
	   var ixScanExplain = dbcl_IndexScan.find(randomConditionObj,null).explain();
	   var ixScanExplainRecs = [];
	   while( ixScanExplain.next() )
	   {
			ixScanExplainRecs.push( ixScanExplain.current().toObj() );
	   }
	   
	   //get table scan result
	   var tbScanCursor = dbcl_IndexScan.find( randomConditionObj, null ).hint({"":null}).sort( { _id: 1 } );
	   
	   //get table scan explain
	   var tbScanExplain = dbcl_IndexScan.find(randomConditionObj,null).hint({"":null}).explain();
	   var tbScanExplainRecs = [];
	   while( tbScanExplain.next() )
	   {
			tbScanExplainRecs.push( tbScanExplain.current().toObj() );
	   }
	   
	   //check count
      if( ixScanCursor.count().toString() !== tbScanCursor.count().toString() )
      {
         println("ixScanCursor.count():"+ixScanCursor.count());
         println("tbScanCursor.count():"+tbScanCursor.count());
      	println("\n\nixScanExplainRecs:"+JSON.stringify(ixScanExplainRecs));
      	println("\n\ntbScanExplainRecs:"+JSON.stringify(tbScanExplainRecs));
      	throw buildException("check count", null, "", ixScanCursor.count(), tbScanCursor.count());
      }
      
      //check every records every fields,tbScanRecs as compare source
      while( ixScanCursor.next() && tbScanCursor.next() )
      {
   		var ixScanRec = ixScanCursor.current().toObj();
   		delete ixScanRec["_id"];
   		var tbScanRec = tbScanCursor.current().toObj();
   		delete tbScanRec["_id"];
   		if( JSON.stringify(ixScanRec) !== JSON.stringify(tbScanRec) )
			{
				//println("\n\nallDataRecs:" + JSON.stringify(allDataRecs));
				println("\n\nixScanExplainRecs:"+JSON.stringify(ixScanExplainRecs));
				println("\n\ntbScanExplainRecs:"+JSON.stringify(tbScanExplainRecs));
				throw buildException("check record", null, JSON.stringify(ixScanRec),JSON.stringify(tbScanRec));
			}
      }
   }
}
