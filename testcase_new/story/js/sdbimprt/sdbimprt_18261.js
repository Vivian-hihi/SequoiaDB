/************************************************************************
*@Description:       1.导入时指定字段分隔符-e包含记录分隔符-r，检查结果
                     2.导入时指定字符串分隔符-a包含记录分隔符-r，检查结果
                     3.导入时指定字段分隔符-e包含字符串分隔符-a，检查结果
                   seqDB-18261
*@Author:   2019-4-18  chensiqin
************************************************************************/

main() ;

function main()
{  
   try
   {
      var csName = COMMCSNAME;
      var clName = COMMCLNAME+"_18261" ;
      var cl = readyCL( csName, clName );
      
      var imprtFile = tmpFileDir +"18261.csv";
      importData( csName, clName, imprtFile , cl);
      cleanCL( csName, clName );
   }
   catch(e)
   {
      throw e;
   }
}

function readyData( imprtFile, content )
{
   println("\n---Begin to ready data.");
   
   var file = fileInit( imprtFile );
   file.write( content );
   var fileInfo = cmd.run( "cat "+ imprtFile );
   println( imprtFile +"\n" + fileInfo );
   file.close();
}

function importData( csName, clName, imprtFile, cl )
{
   println("\n---Begin to import data and check exec result.");
   readyData( imprtFile, "c,YdY1,Y\"exprtTest\"Y" );
   var imprtOption = installDir +"bin/sdbimprt -s "+ COORDHOSTNAME +" -p "+ COORDSVCNAME 
                     +" -c "+ csName +" -l "+ clName 
                     +" --type csv -e ',Y' -r 'Y' --headerline true --fields='c int,d string'"
                     +" --file "+ imprtFile;
   
   try
   {
     println( imprtOption );
     var rc = cmd.run( imprtOption ); //TODO:rc返回值没有用到
     throw buildException( "importData", null, "[sdbimprt results]", 
                        "expected thow exception", 
                        "actual success" );  //TODO:为什么不使用rc返回的报错信息？
   }
   catch(e)
   { //TODO: 这样写不合理吧？非预期的其他错误/异常不用抛出来吗？
   }
   cl.truncate();
   cmd.run( "rm -rf " + imprtFile );
   
   // TODO：另外，多个测试点建议分开写多个方法吧，如importDataER/importDataAR/importDataEA
   readyData( imprtFile, "c,dD1,YDexprtTestYDD2,YDexprtTest2YDD" );
   var imprtOption = installDir +"bin/sdbimprt -s "+ COORDHOSTNAME +" -p "+ COORDSVCNAME 
                     +" -c "+ csName +" -l "+ clName 
                     +" --type csv -a 'YD' -r 'D' --headerline true --linepriority false --fields='c int,d string'"
                     +" --file "+ imprtFile;
   
   testRunCommand(imprtOption);
   checkCLData( cl );
   cl.truncate();
   cmd.run( "rm -rf " + imprtFile );
   
   readyData( imprtFile, "cYDd\n1YDDexprtTestD\n2YDDexprtTest2D" );
   var imprtOption = installDir +"bin/sdbimprt -s "+ COORDHOSTNAME +" -p "+ COORDSVCNAME 
                     +" -c "+ csName +" -l "+ clName 
                     +" --type csv -e 'YD' -a 'D' --headerline true --fields='c int,d string'"
                     +" --file "+ imprtFile;
   
   try
   {
     println( imprtOption );
     var rc = cmd.run( imprtOption );
     throw buildException( "importData", null, "[sdbimprt results]", 
                        "expected thow exception", 
                        "actual success" );
   }
   catch(e)
   {
   }
   cl.truncate();
   cmd.run( "rm -rf " + imprtFile );
   
}

function testRunCommand(command)
{
   println( command );
   var rc = cmd.run( command );
   println( rc );
   
    //check import results
    var rcObj = rc.split("\n");
    var expParseRecords    = "parsed records: 2";
    var expImportedRecords = "imported records: 2";
    var actParseRecords    = rcObj[0];
    var actImportedRecords = rcObj[4];
    if( expParseRecords !== actParseRecords || expImportedRecords !== actImportedRecords )
    {
        throw buildException( "importData", null, "[sdbimprt results]", 
                        "["+ expParseRecords +", "+ expImportedRecords +"]", 
                        "["+ actParseRecords +", "+ actImportedRecords +"]" );
    }
}

function checkCLData( cl )
{
   println("\n---Begin to check cl data.");
   
   var rc = cl.find();
   var recsArray = [];
   while( tmpRecs = rc.next() )
   {
      recsArray.push( tmpRecs.toObj() );
   }
   
   var expCnt  = 2;
   var actCnt  = recsArray.length;
   if( actCnt !== expCnt )
   {
      throw buildException( "checkCLdata", null, "[find]", 
                        "[cnt:"+ expCnt +"]", 
                        "[cnt:"+ actCnt +"]" );
   }
   
}