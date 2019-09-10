/************************************************************************
*@Description:  seqDB-19168:科学计数法，底数为整数（如 1E+308）
*@Author     :  2019-8-21  huangxiaoni
************************************************************************/

//main(); 
function main()
{  
   var seqDBId = '19168_csv';
   var csName = COMMCSNAME;
   var clName = "sdbimprt_"+ seqDBId ; 
   var cl = readyCL( csName, clName ); 
   
   var importFile = tmpFileDir + seqDBId +".csv";
   var exportFile = tmpFileDir +"sdbexprt_"+ seqDBId +".csv";   
   initData( importFile ); 
   /*
   var expRecsNum = 73; 
   var expImportRecs = initExpImportData();
   var expExportRecs = initExpExportData();
   var type = 'csv';
   
   // sdbimprt   
   var importFields = 'a int, b'
   var rcResults = importData( csName, clName, importFile, type, importFields );   
   checkImportRC( rcResults, expRecsNum );  
   checkCLData( cl, expRecsNum, expImportRecs );
   
   // sdbexprt   
   var exportFields = 'a, b';
   var sort = '{a:1}';
   var otherParam = ' --floatfmt %.16g';
   exportData( csName, clName, exportFile, type, exportFields, sort, otherParam );
   checkExportData( exportFile, expExportRecs );
   
   cleanCL( csName, clName );  */
}

function initImportFile( importFile )
{
   println("\n---Begin to ready data for sdbimport.");
   var baseNum = 1;
   var indexNum = 308;
   var str = "";
   while ( indexNum > 0 )
   {
      str += baseNum + "E" + indexNum + "\n";
      baseNum++;
      indexNum--;
   }
   
   var file = fileInit( importFile );
   file.write( str );   
      
   file.close();
}

function initExpImportData()
{   
   println("\n---Begin to ready expect data for sdbimprt.");
   
   var expB1 = '3.14';
   var expRecs = '[]';
   
   return expRecs;
}

function initExpExportData()
{   
   println("\n---Begin to ready expect data for sdbexprt.");   
   
   var expB1 = '3.14';
   var expRecs = '[]';
   
   return expRecs;
}