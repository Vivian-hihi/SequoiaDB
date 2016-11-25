/************************************
*@Description: random data,random matches,find data,only check count
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
   
   try
   {
      //insert random numberical data
      var fieldNames = ['fieldName1','fieldName2','fieldName3','fieldName4','fieldName5','fieldName6'];
      
      for(var i= 0; i< 200;i++){
         var rd = new dataGenerator();
         var recs = rd.getRecords( 10000, ["int", "long", "float", "array"], fieldNames );
         insertData(dbcl, recs);
      }
      
      var loopNum = 1;
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
   		
   	   //get index scan result
   	   var ixScanCursor = dbcl.find( randomConditionObj, null ).sort( { _id: 1 } );
   	   
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
   	   var tbScanCursor = dbcl.find( randomConditionObj, null ).sort( { _id: 1 } );
   	   
   	   //get table scan explain
   	   var tbScanExplain = dbcl.find(randomConditionObj,null).explain();
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
         /*while( ixScanCursor.next() && tbScanCursor.next() )
         {
      		var ixScanRec = ixScanCursor.current().toObj();
      		var tbScanRec = tbScanCursor.current().toObj();
      		println("\nixScanRec = "+JSON.stringify(ixScanRec)+"\n\ntbScanRec = "+JSON.stringify(tbScanRec));
      		if( JSON.stringify(ixScanRec) !== JSON.stringify(tbScanRec) )
   			{
   				//println("\n\nallDataRecs:" + JSON.stringify(allDataRecs));
   				println("\n\nixScanExplainRecs:"+JSON.stringify(ixScanExplainRecs));
   				println("\n\ntbScanExplainRecs:"+JSON.stringify(tbScanExplainRecs));
   				println("\nixScanRec = "+JSON.stringify(ixScanRec)+"\n\ntbScanRec = "+JSON.stringify(tbScanRec));
   				throw buildException("check record", "rec ERROR");
   			}
         }*/
      }
   }catch(e){
      throw e;
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
