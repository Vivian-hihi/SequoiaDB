/***************************************************************************
@Description :seqDB-14370 :查询全文索引    
@Modify list :
              2018-10-26  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_14370";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   dbcl.createIndex( "fullIndex", {content : "text"});
   
   var indexes = dbcl.listIndexes();
   var actIndexes = new Array();
   while(indexes.next()){
      var index = indexes.current().toObj();
      actIndexes.push(index);
   }
   
   //获取预期的索引结果
   var dbOperator = new DBOperator();
   var cappedCLName = dbOperator.getCappedCLName( dbcl, "fullIndex" );
   var expIndexes = getIndexes(cappedCLName);
   
   checkIndexes(expIndexes, actIndexes);
   
   //get全文索引
   var index = dbcl.getIndex("fullIndex");
   var actIndexes = new Array();
   actIndexes.push(index.toObj());
   var expIndexes = getIndexes(cappedCLName);
   expIndexes.pop();
   
   checkIndexes(expIndexes, actIndexes);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function getIndexes(cappedCLName){
   var arrayIndexes = new Array();
   var index = { "IndexDef": { "name": "fullIndex", "key": { "content": "text" }, "v": 0, "unique": false, "dropDups": false, "enforced": false }, "IndexFlag": "Normal", "Type": "Text", "ExtDataName": cappedCLName };
   arrayIndexes.push(index);
   var index = { "IndexDef": { "name": "$id", "key": { "_id": 1 }, "v": 0, "unique": true, "dropDups": false, "enforced": true }, "IndexFlag": "Normal", "Type": "Positive" };
   arrayIndexes.push(index);
   return arrayIndexes;
}

function checkIndexes(expIndexes, actIndexes){
   if(expIndexes.length !== actIndexes.length)
   {
      throw buildException("checkResult()", "check records", "check records length", expIndexes.length, actIndexes.length);
   }
   expIndexes.sort(compare("Type"));
   actIndexes.sort(compare("Type"));
   
   // compare array  
   for( var i in expIndexes )
   {
      var actRec = actIndexes[i];
      var expRec = expIndexes[i];
   	
      for ( var f in expRec )
      {
		 if(f == "IndexDef"){
		    for (var j in expRec[f]){
			   if(JSON.stringify(actRec[f][j]) !== JSON.stringify(expRec[f][j])){
			      println("exp : " + JSON.stringify(expRec[f][j]) + " act : " + JSON.stringify(actRec[f][j]));
			      throw buildException("checkResult()", "check record fail", "fail", JSON.stringify(JSON.stringify(expRec)), JSON.stringify(actRec));
			   }
			}
			continue;
		 }
         if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) ) 
         {
			println("exp : " + JSON.stringify(expRec[f]) + " act : " + JSON.stringify(actRec[f]));
            throw buildException("checkResult()", "check record fail", "fail", JSON.stringify(JSON.stringify(expRec)), JSON.stringify(actRec));
         }
      }
   }
   println("check results success!");
}

main()