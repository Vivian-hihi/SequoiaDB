/************************************
*@Description:  SampleRecords取不同值执行统计   
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11757
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11757";
   commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
   //create cl	
   var clName = COMMCLNAME + "11757";
   var dbcl = commCreateCL( db, csName, clName );
	
   //insert datas
   var insertNums = 150;
   insertDiffDatas( dbcl, insertNums );
   
   commCreateIndex( dbcl, "a", {a : 1}, false );
	                                                                    
   //analyze with SampleNum
   var options = {CollectionSpace: csName, SampleNum: 200};
   analyze( db, options );
                                                                                       
   //check SampleNum
   var expResult = { "SampleRecords": 150, "TotalRecords": 150 };
   checkInfoState( csName, clName, expResult );
   
   //insert datas again
   insertDiffDatas( dbcl, insertNums );
   
   //analyze again with SampleNum
   var options = {CollectionSpace: csName, SampleNum: 150};
   analyze( db, options );
   
   //check SampleNum again 
   var expResult = { "SampleRecords": 150, "TotalRecords": 300 };
   checkInfoState( csName, clName, expResult );
   
   //analyze again without SampleNum
   var options = {CollectionSpace: csName};
   analyze( db, options );
   
   //check SampleNum again 
   var expResult = { "SampleRecords": 200, "TotalRecords": 300 };
   checkInfoState( csName, clName, expResult );
   
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkInfoState( csName, clName, expResult )
{  
   try
   {
      var dataDB = getDataDB( csName, clName );
      //check collection state
      var matcher = { "CollectionSpace": csName, "Collection": clName };
      
      var actResult = dataDB.SYSSTAT.SYSCOLLECTIONSTAT.find(matcher);
      //println("actResult: " + actResult);
      
      if(0 < actResult.length)
      {
         var expSampleNum = expResult["SampleRecords"];
         var expTotalRecord = expResult["TotalRecords"];
         
         var actSampleNum = actResult[0]["SampleRecords"];
         var actTotalRecord = actResult[0]["TotalRecords"];
         
         if(expSampleNum !== actSampleNum && expTotalRecord !== actTotalRecord)
         {
            println("expSampleNum: " + expSampleNum + "actSampleNum " + actSampleNum + "expTotalRecord: " + expTotalRecord + "actTotalRecord " + actTotalRecord);
            throw "check failed";
         }
      }
      
      //check index state
      actResult = dataDB.SYSSTAT.SYSINDEXSTAT.find(matcher);
      if(0 < actResult.length)
      {
         var expSampleNum = expResult["SampleRecords"];
         var expTotalRecord = expResult["TotalRecords"];
         
         var actSampleNum = actResult[0]["SampleRecords"];
         var actTotalRecord = actResult[0]["TotalRecords"];
         
         if(expSampleNum !== actSampleNum && expTotalRecord !== actTotalRecord)
         {
            println("expSampleNum: " + expSampleNum + "actSampleNum " + actSampleNum + "expTotalRecord: " + expTotalRecord + "actTotalRecord " + actTotalRecord);
            throw "check failed";
         }
      }
   }
   catch(e)
   {
      throw buildException("check state", e, "check", "check success", e);
   }
}

function getDataDB( csName, clName )
{  
   if (commIsStandalone(db))
   {  
      return db;
   }
	
   var groupName = commGetCLGroups( db, csName + "." + clName );
   var groupDetail = commGetGroups( db, false, groupName );
                     
   var priNodePos = groupDetail[0][0].PrimaryPos;
                      	
   var hostName = groupDetail[0][priNodePos].HostName;
   var svcName = groupDetail[0][priNodePos].svcname;
   
   return new Sdb(hostName, svcName);
}

main();
