/************************************
*@Description: limit/skip,limit + skip > record num
*@author:      zhaoyu
*@createdate:  2018.10.12
**************************************/
function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Deploy mode is standalone!" );
      return;
   }
   
   var clName = COMMCLNAME + "_ES_15130";
   var clFullName = COMMCSNAME + "." + clName
   var indexName = "a";
   
   commDropCL( db, COMMCSNAME, clName);
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, indexName, {a:"text"});
   
   //insert random record
   for(var i=0; i<3; i++)
   {
      var rd = new commDataGenerator();
      var recs = rd.getRecords( 10000, "string", ['a', 'b'] );
      insertRecords(dbcl, recs);
   }
   
   var recordNum = parseInt(dbcl.count());
   if(recordNum !== 30000)
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return;
   }
   
   var esOperator = new ESOperator();
   var dbOperator = new DBOperator();
   var eSIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, indexName);
   checkFullSyncToES(COMMCSNAME, clName, indexName, 30000);
   
   var expectRecords = dbOperator.findFromCL(dbcl, null, null, {_id:1}, null, 15000, 16000);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}}, null, {_id:1}, null, 15000, 16000);
   checkResult(expectRecords, actRecords);
   println("---check skip<recordNum,limit<recordNum success---");
   
   var expectRecords = dbOperator.findFromCL(dbcl, null, null, {_id:1}, null, 15000, 40000);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}}, null, {_id:1}, null, 15000, 40000);
   checkResult(expectRecords, actRecords);
   println("---check skip>recordNum,limit<recordNum,skip+limit>1w success---");
   
   var expectRecords = dbOperator.findFromCL(dbcl, null, null, {_id:1}, null, 50000, 40000);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}}, null, {_id:1}, null, 50000, 40000);
   checkResult(expectRecords, actRecords);
   println("---check skip>recordNum,limit>recordNum success---");
   
   var expectRecords = dbOperator.findFromCL(dbcl, null, null, {_id:1}, null, 40000, 12000);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}}, null, {_id:1}, null, 40000, 12000);
   checkResult(expectRecords, actRecords);
   println("---check skip<recordNum,limit>recordNum success---");
   
   commDropCL( db, COMMCSNAME, clName);
}
main()