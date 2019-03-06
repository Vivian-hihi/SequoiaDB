/*******************************************************************************
*@Description:   seqDB-12797: sdbimprt支持strictfieldnum配置 
*@Author:        2019-3-5  wangkexin
********************************************************************************/

var csvContent1 = 'test|123\ntest1|456' + "\n" ;
var csvContent2 = 'test|\ntest1|123\ntest2|456' + "\n" ;
var csvContent3 = 'test|123\ntest1|\ntest2|456' + "\n" ;
var csvContent4 = 'test|123\ntest1|456\ntest2|' + "\n" ;
main();

function main()
{  
   try
   {
      var csName = COMMCSNAME;
      var clName = COMMCLNAME+"_12797" ;
      var cl = readyCL( csName, clName );
      
	  //准备数据文件
      var imprtFile1 = tmpFileDir +"12797a.csv";
      readyData( imprtFile1, csvContent1);
	  var imprtFile2 = tmpFileDir +"12797b.csv";
      readyData( imprtFile2, csvContent2);
	  var imprtFile3 = tmpFileDir +"12797c.csv";
      readyData( imprtFile3, csvContent3);
	  var imprtFile4 = tmpFileDir +"12797d.csv";
      readyData( imprtFile4, csvContent4);
	  
	  //1. strictfieldnum=true，字段数与定义一致
      importData( csName, clName, imprtFile1, 2, 0, 2, true);
	  var expRecs = '[{"yourname":"test","yourid":123},{"yourname":"test1","yourid":456}]';
	  checkCLData( cl, expRecs, 2);
	  cl.truncate();
	  //2. strictfieldnum=true，字段数与定义不一致，覆盖不一致出现位置为首部，中部以及尾部
      importData( csName, clName, imprtFile2, 2, 1, 2, true);
	  var expRecs = '[{"yourname":"test1","yourid":123},{"yourname":"test2","yourid":456}]';
	  checkCLData( cl, expRecs, 2);
	  cl.truncate();
	  /*//3. 指定headerline=true，field字段，指定与headerline不同的分隔符
      importData( csName, clName, imprtFile3, true, 3, 0, 3, ',');
	  var expRecs = '[{"yourname":"test1","yourid":123},{"yourname":"test2","yourid":123},{"yourname":"test3","yourid":123}]';
	  checkCLData( cl, expRecs, 3);
	  cl.truncate();
	  //4. 指定headerline=false，field字段，不指定分隔符
      importData( csName, clName, imprtFile1, false, 3, 1, 3 );
	  var expRecs = '[{"yourname":"test1","yourid":123},{"yourname":"test2","yourid":123},{"yourname":"test3","yourid":123}]';
	  checkCLData( cl, expRecs, 3);
	  cl.truncate();
	  //5. 指定headerline=false，field字段，指定相同分隔符
      importData( csName, clName, imprtFile2, false, 3, 1, 3, '|');
	  var expRecs = '[{"yourname":"test1","yourid":123},{"yourname":"test2","yourid":123},{"yourname":"test3","yourid":123}]';
	  checkCLData( cl, expRecs, 3);
	  cl.truncate();
	  //6. 指定headerline=false，field字段，指定与headerline不同的分隔符
      importData( csName, clName, imprtFile3, false, 4, 0, 4, ',');
	  var expRecs = '[{"yourname":"NAME|ID"},{"yourname":"test1","yourid":123},{"yourname":"test2","yourid":123},{"yourname":"test3","yourid":123}]';
	  checkCLData( cl, expRecs, 4);*/
      
      cleanCL( csName, clName );
   }
      catch(e)
   {
   	throw e;
   }
}

function readyData( imprtFile, csvContent )
{
   println("---Begin to ready data.");
   
   var file = fileInit( imprtFile );
   file.write(csvContent);
   var fileInfo = cmd.run( "cat "+ imprtFile );
   println( imprtFile +"\n" + fileInfo );
   file.close();
}

function importData( csName, clName, imprtFile, expParseRecordsNum, expParseFailureNum, expImportedRecordsNum, strictfieldnum )
{
   println("\n---Begin to import data and check exec result.");
   
   //remove rec file
   var tmpRec = csName +"_"+ clName +"*.rec";
   cmd.run( "rm -rf "+ tmpRec );
   
   //import operation
   if ( typeof strictfieldnum == "undefined" )
   {
	   var imprtOption = installDir +'bin/sdbimprt -s '+ COORDHOSTNAME +' -p '+ COORDSVCNAME 
                     +' -c '+ csName +' -l '+ clName 
                     +' --type csv '
                     +' --file '+ imprtFile
					 +' --fields "yourname string,yourid int"'
					 +' -e "|"';
					 
   }
   else
   {
	   var imprtOption = installDir +'bin/sdbimprt -s '+ COORDHOSTNAME +' -p '+ COORDSVCNAME 
                     +' -c '+ csName +' -l '+ clName 
                     +' --type csv '
                     +' --file '+ imprtFile
					 +' --fields "yourname string,yourid int"'
					 +' -e "|"'
					 +'	--strictfieldnum ' + strictfieldnum;
   }
   println( imprtOption );
   var rc = cmd.run( imprtOption );
   println( rc );
   
   //check import results
   var rcObj = rc.split("\n");
   var expParseRecords    = "parsed records: "+expParseRecordsNum;
   var expParseFailure    = "parse failure: "+expParseFailureNum;
   var expImportedRecords = "imported records: "+expImportedRecordsNum;
   var actParseRecords    = rcObj[0];
   var actParseFailure    = rcObj[1];
   var actImportedRecords = rcObj[4];
   if( expParseRecords !== actParseRecords 
    || expParseFailure !== actParseFailure
	|| expImportedRecords !== actImportedRecords)
   {
      throw buildException( "importData", null, "[sdbimprt results]", 
                        "["+ expParseRecords +", "+ expImportedRecords +"]", 
						"["+ expParseFailure +", "+ actParseFailure +"]", 
                        "["+ actParseRecords +", "+ actImportedRecords +"]" );
   }
    
   // clean tmpRec
   cmd.run( "rm -rf " + tmpRec );
}

function checkCLData( cl, expRecs, expCnt )
{
   println("---Begin to check cl data.");
   
   var rc = cl.find({},{_id:{$include:0}});
   var recsArray = [];
   while( tmpRecs = rc.next() )
   {
      recsArray.push( tmpRecs.toObj() );
   }
   
   var actCnt  = recsArray.length;
   var actRecs = JSON.stringify( recsArray );
   if( actCnt !== expCnt || actRecs !== expRecs )
   {
      throw buildException( "checkCLdata", null, "[find]", 
                        "[cnt:"+ expCnt +", recs:"+ expRecs +"]", 
                        "[cnt:"+ actCnt +", recs:"+ actRecs +"]" );
   }
   println( "cl records: "+ actRecs );
}