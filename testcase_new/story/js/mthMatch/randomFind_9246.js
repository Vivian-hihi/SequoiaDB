/************************************
*@Description: random data,random matches,find data
*@author:      zhaoyu
*@createdate:  2016.11.3
*@testlinkCase:seqDB-9246 
**************************************/
function main()
{
   //clean environment before test
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,"drop CL in the beginning" ) ;
   
   //create cl
   var dbcl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0);
   
   //insert random numberical data
   var fieldNames = ['fieldName1','fieldName2','fieldName3','fieldName4','fieldName5','fieldName6'];
   var rd = new dataGenerator();
   var recs = rd.getRecords( 2000, ["int", "long", "float", "array"], fieldNames );
   insertData(dbcl, recs);
   
   var loopNum = 30;
   for(var i= 0; i< loopNum;i++){
	   //create index
	   commCreateIndex( dbcl, "fieldName1Index", {fieldName1:1});
	   commCreateIndex( dbcl, "fieldName2Index", {fieldName2:-1});
	   commCreateIndex( dbcl, "fieldName3Index", {fieldName3:1});
	   commCreateIndex( dbcl, "fieldName4Index", {fieldName4:-1});
	   commCreateIndex( dbcl, "fieldName5Index", {fieldName5:1});
	   commCreateIndex( dbcl, "fieldName6Index", {fieldName6:-1});
	   
	   //set all matches support index scan
	   matches1 = [ "$et", "$gt", "$gte", "$lt", "$lte", "$ne" ];
	   matches2 = [ "$in", "$all" ];
	   
	   var findCondition1 = getFindCondition( 1, ["int", "long", "float" ], matches1 );
	   var findValue2 = getRandomArray();
	   var findCondition2 = getFindCondition( 1, "array", matches2 );
	   
	   findCondition3 = [ {$exists:0}, {$isnull:1} ];
	   
	   //convert array to object
	   findConditionObj1 = arrToObj( findCondition1 );
	   findConditionObj2 = arrToObj( findCondition2 );
	   findConditionObj3 = arrToObj( findCondition3 );
	   
	   var obj1 = mergeObj(findConditionObj1, findConditionObj2);
	   var obj2 = mergeObj(obj1, findConditionObj3);
	   
	   var findConditions = [];
	   var cnt = 0;
	   for( var j in obj2 ){ 
		  var subcond = {};
		  subcond[j] = obj2[j]
		  findConditions.push(subcond);
	   }
	   
	   //generate random find condition
	   var randomCondition = genRandomFindCondition( findConditions, fieldNames );
	   var randomConditionObj = {$and:randomCondition};
	   println("randomCondition:"+ JSON.stringify(randomConditionObj));
	   
	   //get all data;
	   var allData = dbcl.find().sort( { No: 1 } );
	   var allDataRecs = [];
	   while( allData.next() )
	   {
			allDataRecs.push( allData.current().toObj() );
	   }
		   
	   //get index scan result
	   var ixScanCursor = dbcl.find( randomConditionObj, null ).sort( { No: 1 } );
	   
	   //get index scan explain
	   var ixScanExplain = dbcl.find(randomConditionObj,null).explain();
	   var ixScanExplainRecs = [];
	   while( ixScanExplain.next() )
	   {
			ixScanExplainRecs.push( ixScanExplain.current().toObj() );
	   }
	   
	   //drop index
	   commDropIndex(dbcl, "fieldName1Index");
	   commDropIndex(dbcl, "fieldName2Index");
	   commDropIndex(dbcl, "fieldName3Index");
	   commDropIndex(dbcl, "fieldName4Index");
	   commDropIndex(dbcl, "fieldName5Index");
	   commDropIndex(dbcl, "fieldName6Index");
	   
	   //get table scan result
	   var tbScanCursor = dbcl.find( randomConditionObj, null ).sort( { No: 1 } );
	   
	   //get table scan explain
	   var tbScanExplain = dbcl.find(randomConditionObj,null).explain();
	   var tbScanExplainRecs = [];
	   while( tbScanExplain.next() )
	   {
			tbScanExplainRecs.push( tbScanExplain.current().toObj() );
	   }
	   
	   //compare index scan and table scan to check result
	   var ixScanRecs = [];
	   while( ixScanCursor.next() )
	   {
			ixScanRecs.push( ixScanCursor.current().toObj() );
	   }
	   
	   //get expect records to array
	   var tbScanRecs = [];
	   while( tbScanCursor.next() )
	   {
			tbScanRecs.push( tbScanCursor.current().toObj() );
	   }
	   //check count
	   if( ixScanRecs.length !== tbScanRecs.length )
	   {
		println("\n\nallDataRecs:" + JSON.stringify(allDataRecs));
		println("\n\nixScanExplainRecs:"+JSON.stringify(ixScanExplainRecs));
		println("\n\ntbScanExplainRecs:"+JSON.stringify(tbScanExplainRecs));
		println("\nindex scan get records = "+JSON.stringify(ixScanRecs)+"\n\ntable scan get records = "+JSON.stringify(tbScanRecs));
		throw buildException("check count", null, "",
										tbScanRecs.length, ixScanRecs.length);
	   }
	   
	   //check every records every fields,tbScanRecs as compare source
	   for( var j in tbScanRecs )
	   {
			var ixScanRec = ixScanRecs[j];
			var tbScanRec = tbScanRecs[j];
			
			for ( var f in tbScanRec )
			{
				if( JSON.stringify(ixScanRec[f]) !== JSON.stringify(tbScanRec[f]) )
				{
					println("\n\nallDataRecs:" + JSON.stringify(allDataRecs));
					println("\n\nixScanExplainRecs:"+JSON.stringify(ixScanExplainRecs));
					println("\n\ntbScanExplainRecs:"+JSON.stringify(tbScanExplainRecs));
					println("\nerror occurs in "+(parseInt(j)+1)+"th record, in field '"+f+"'");
					println("\nixScanRec = "+JSON.stringify(ixScanRec)+"\n\ntbScanRec = "+JSON.stringify(tbScanRec));
					println("\nindex scan get records= "+JSON.stringify(ixScanRecs)+"\n\ntable scan get records = "+JSON.stringify(tbScanRecs)); 
					throw buildException("checkRec()", "rec ERROR");
				}
			}
	   }
	}
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
   for(var i in newArr){
      for (var k in newArr[i])
      {
         obj[k] = newArr[i][k];
      } 
   }
   return obj;
}

function arrToObj( arr ){
   var obj = {};
   for(var i in arr){
      for (var k in arr[i])
      {
         obj[k] = arr[i][k];
      } 
   }
   return obj;
}
