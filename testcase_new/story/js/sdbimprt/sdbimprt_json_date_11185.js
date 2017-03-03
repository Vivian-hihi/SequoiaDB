/************************************************************************
*@Description:  seqDB-11185:导入date类型数据
*@Author:            2017-3-1  huangxiaoni
************************************************************************/
main();

function main()
{  
   try
   {
      var csName = COMMCSNAME;
      var clName = COMMCLNAME+"_11185" ;
      var cl = readyCL( csName, clName );
      
      var imprtFile = testCaseDir +"dataFile/date.json";
      var exprtFile = tmpFileDir +"sdbexprt_11185.csv";
      
      importData( csName, clName, imprtFile );
      checkCLData( cl );
      
      exprtData( csName, clName, exprtFile );
      checkExprtFile( csName, clName, exprtFile );
      
      cleanCL( csName, clName );
   }
      catch(e)
   {
   	throw e;
   }
}

function importData( csName, clName, imprtFile )
{
   println("\n---Begin to import data and check exec result.");
   
   //remove rec file
   var tmpRec = csName +"_"+ clName +"*.rec";
   cmd.run( "rm -rf "+ tmpRec );
   
   var imprtOption = installDir +'bin/sdbimprt -s '+ COORDHOSTNAME +' -p '+ COORDSVCNAME 
                     +' -c '+ csName +' -l '+ clName 
                     +' --type json'
                     +' --file '+ imprtFile;
   println( imprtOption );
   var rc = cmd.run( imprtOption );
   println( rc );
   
  //check import results
   var rcObj = rc.split("\n");
   var expParseRecords    = "parsed records: 34";
   var expParseFailure    = "parse failure: 33";
   var expImportedRecords = "imported records: 34";
   var actParseRecords    = rcObj[0];
   var actParseFailure    = rcObj[1];
   var actImportedRecords = rcObj[4];
   if( expParseRecords !== actParseRecords || expParseRecords !== actParseRecords 
    || expImportedRecords !== actImportedRecords )
   {
      throw buildException( "importData", null, "[sdbimprt results]", 
                        "["+ expParseRecords +", "+ expParseFailure +", "+ expImportedRecords +"]", 
                        "["+ actParseRecords +", "+ actParseFailure +", "+ actImportedRecords +"]" );
   }
   
   //check failed records
   var rec = cmd.run( "ls "+ tmpRec ).split("\n")[0];
   var actFailedNum = cmd.run( "cat "+ rec ).split("\n").length - 1;
   println( rec +"\nrecords number: "+ actFailedNum );
   var expFailedNum = 33*2+3;
   if( expFailedNum !== actFailedNum )
   {
      throw buildException( "checkCLdata", null, "[find]", 
                        "[failedRecs:"+ expFailedNum +"]", 
                        "[failedRecs:"+ actFailedNum +"]" );
   }
}

function checkCLData( cl )
{
   println("\n---Begin to check cl data.");
   
   var rc = cl.find({$and:[{b:{$type:1,$et:9}},{a:{$ne:50}}]},{_id:{$include:0}}).sort({a:1}); //except '{ a:50, b: SdbDate() }'
   var recsArray = [];
   while( tmpRecs = rc.next() )
   {
      recsArray.push( tmpRecs.toObj() );
   }
   
   var expCnt  = 33;  
   var expRecs = '[{"a":0,"b":{"$date":"1900-01-01"}},{"a":1,"b":{"$date":"1970-01-01"}},{"a":2,"b":{"$date":"9999-12-31"}},{"a":3,"b":{"$date":-62135596800000}},{"a":4,"b":{"$date":-2209075200000}},{"a":5,"b":{"$date":"1900-01-01"}},{"a":6,"b":{"$date":"9999-12-31"}},{"a":7,"b":{"$date":-62135625600000}},{"a":8,"b":{"$date":-2209104000000}},{"a":9,"b":{"$date":"1900-01-01"}},{"a":10,"b":{"$date":"9999-12-31"}},{"a":11,"b":{"$date":-9223372036854776000}},{"a":12,"b":{"$date":-9223372036854776000}},{"a":13,"b":{"$date":9223372036854776000}},{"a":14,"b":{"$date":9223372036854776000}},{"a":15,"b":{"$date":-62135625957000}},{"a":16,"b":{"$date":"1900-01-01"}},{"a":17,"b":{"$date":"9999-12-31"}},{"a":25,"b":{"$date":-9223372036854776000}},{"a":27,"b":{"$date":9223372036854776000}},{"a":29,"b":{"$date":-62135625957000}},{"a":30,"b":{"$date":"1900-01-01"}},{"a":31,"b":{"$date":"9999-12-31"}},{"a":51,"b":{"$date":"1901-01-01"}},{"a":52,"b":{"$date":"9999-12-31"}},{"a":53,"b":{"$date":-62135596800000}},{"a":54,"b":{"$date":-2209075200000}},{"a":55,"b":{"$date":"1900-01-01"}},{"a":56,"b":{"$date":"9999-12-31"}},{"a":57,"b":{"$date":-62135625600000}},{"a":58,"b":{"$date":-2209104000000}},{"a":59,"b":{"$date":"1900-01-01"}},{"a":60,"b":{"$date":"9999-12-31"}}]';
   var actCnt  = recsArray.length;
   var actRecs = JSON.stringify( recsArray );
   if( actCnt !== expCnt || actRecs !== expRecs )
   {
      throw buildException( "checkCLdata", null, "[find]", 
                        "[cnt:"+ expCnt +", recs:"+ expRecs +"]\n", 
                        "[cnt:"+ actCnt +", recs:"+ actRecs +"]" );
   }
   //println( "cl records: "+ actRecs );
   
}
   
function exprtData( csName, clName, exprtFile )
{
   println("\n---Begin to export data.");
   
   //remove export file
   cmd.run( "rm -rf "+ exprtFile );
   
   //export operation
   var exportOption = installDir +'bin/sdbexprt -s '+ COORDHOSTNAME +' -p '+ COORDSVCNAME 
                     +' -c '+ csName +' -l '+ clName 
                     +' --type json --fields "a,b"'
                     +' --sort "{a:1}" --file '+ exprtFile;
   println( exportOption );
   var rc = cmd.run( exportOption );
   println( rc );
   
   //cat exprt file
   var fileInfo = cmd.run( "cat "+ exprtFile );
   println( exprtFile +"\\n" + fileInfo );
}

function checkExprtFile( csName, clName, exprtFile )
{
   println("\n---Begin to check export file data.");
   
   var rcObj = cmd.run( "cat "+ exprtFile ).split("\n");
   var actRC = JSON.stringify( rcObj );
   
   var cl = db.getCS( csName ).getCL( clName );
   var bValue = cl.find({a:50}).current().toObj().b.$date;
   //var currentDate = cmd.run('date "+%Y-%m-%d"').split("\n")[0];
   //println( bValue );
   
   var expRC = '["{ \\"a\\": 0, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 1, \\"b\\": { \\"$date\\": \\"1970-01-01\\" } }","{ \\"a\\": 2, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 3, \\"b\\": { \\"$date\\": -62135596800000 } }","{ \\"a\\": 4, \\"b\\": { \\"$date\\": -2209075200000 } }","{ \\"a\\": 5, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 6, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 7, \\"b\\": { \\"$date\\": -62135625600000 } }","{ \\"a\\": 8, \\"b\\": { \\"$date\\": -2209104000000 } }","{ \\"a\\": 9, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 10, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 11, \\"b\\": { \\"$date\\": -9223372036854775808 } }","{ \\"a\\": 12, \\"b\\": { \\"$date\\": -9223372036854775808 } }","{ \\"a\\": 13, \\"b\\": { \\"$date\\": 9223372036854775807 } }","{ \\"a\\": 14, \\"b\\": { \\"$date\\": 9223372036854775807 } }","{ \\"a\\": 15, \\"b\\": { \\"$date\\": -62135625957000 } }","{ \\"a\\": 16, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 17, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 25, \\"b\\": { \\"$date\\": -9223372036854775808 } }","{ \\"a\\": 27, \\"b\\": { \\"$date\\": 9223372036854775807 } }","{ \\"a\\": 29, \\"b\\": { \\"$date\\": -62135625957000 } }","{ \\"a\\": 30, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 31, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 50, \\"b\\": { \\"$date\\": \\"'+ bValue +'\\" } }","{ \\"a\\": 51, \\"b\\": { \\"$date\\": \\"1901-01-01\\" } }","{ \\"a\\": 52, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 53, \\"b\\": { \\"$date\\": -62135596800000 } }","{ \\"a\\": 54, \\"b\\": { \\"$date\\": -2209075200000 } }","{ \\"a\\": 55, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 56, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }","{ \\"a\\": 57, \\"b\\": { \\"$date\\": -62135625600000 } }","{ \\"a\\": 58, \\"b\\": { \\"$date\\": -2209104000000 } }","{ \\"a\\": 59, \\"b\\": { \\"$date\\": \\"1900-01-01\\" } }","{ \\"a\\": 60, \\"b\\": { \\"$date\\": \\"9999-12-31\\" } }",""]';
   
   if( actRC !== expRC )
   {
      throw buildException( "checkCLdata", null, "[find]", 
                        "[exprtFile data:"+ expRC +"]\n", 
                        "[exprtFile data:"+ actRC +"]" );
   }
   
}