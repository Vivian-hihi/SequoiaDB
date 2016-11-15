/*******************************************************************************
*@Description : import data that the field value is very large.
*               such value: -9223372036854775808/9223372036854775807
*               [about: SEQUOIADBMAINSTREAM-608]
*@Modify list :
*               2015-3-3  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   var FileName = LocalPath + "/" + CHANGEDPREFIX + "_largeValue.file" ;
   var impCmd = InstallPath + "/bin/sdbimprt" ;
   var num1 = "-9223372036854775808";
   var num2 = "9223372036854775807";
   var cmd = new Cmd();
   cmd.run( "rm -rf " + FileName );   // remove file
   var fileObj = new File( FileName ) ;
   fileObj.write( num1 + ",test,中文测试\n" + 
                  num2 + ",test1,中文测试1" ) ;
   var cl = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME,
                                  {ShardingKey:{"acc_no":-1}, ShardingType:"hash"},
                                  true, true, "create collection in the beginning");
   var impCmdOption = "--hostname " + COORDHOSTNAME + " --svcname " + COORDSVCNAME +
                      " -c " + COMMCSNAME + " -l " + COMMCLNAME + " --file " +
                      FileName + " --type csv --fields \"acc_no,context,text\" -n 1 ";
   var clName = COMMCSNAME + "." + COMMCLNAME;
   var clGroup = commGetCLGroups( db, clName );
   var dstGroup = null;
   if( false == commIsStandalone( db ) )
   {
      db.setSessionAttr( {"PreferedInstance":"M"} );
      var groups = commGetGroups( db );
      if( 2 > groups.length )
      {
         println( "WARNNING, the collection have: " + groups.length + " groups" );
         return 0;
      }
      for( var i = 0 ; i < groups.length ; ++i )
      {
         if( clGroup[0] != groups[i][0]["GroupName"] )
         {
            dstGroup = groups[i][0]["GroupName"];
         }
      }
   }
   else
   {
      println( "run mode is standalone" );
      return;
   }
   try
   {
      cmdToolRun( impCmd, impCmdOption );
      println( "success to import data to database" );
      // hash split
      println( "source group: " + clGroup[0] + ", dstGroup: " + dstGroup );
      cl.split( clGroup[0], dstGroup, 50 );
      println( "split success" ) ;
      if( 2 != cl.count() )
      {
         println( "expect records: 2, actural records: " + cl.count() );
         throw "ErrorRecordsNumber";
      }
      // Test Point
      var query1 = cl.find({acc_no: {$numberLong: num1}}).toArray();
      var query2 = cl.find({acc_no: {$numberLong: num2}}).toArray();
      if( 0 == query1.length || 0 == query2.length )
      {
         println( "cl.find({acc_no: {$numberLong: '"+ num1 +"'}}): \n" + query1 );
         println( "cl.find({acc_no: {$numberLong: '"+ num2 +"'}}): \n" + query2 );
         throw "FailedToQueryByMatcher";
      }
      var queryRecords = cl.find().toArray();
      for( var i = 0 ; i < queryRecords.length ; ++i )
      {
         var recordObj = JSON.parse( queryRecords[i] );
         //println( "RECORDS: " + queryRecords[i] );
         var fieldsNum = 0;
         var key = new Array();
         for( var j in recordObj )
         {
            key[fieldsNum] = j;
            fieldsNum++;
         }
         if( 4 != fieldsNum )
         {
            println( "expect fields: 4, actual fields: " + fieldsNum ) ;
            println( "fields key: " + key );
            throw "ErrorFieldsNumber";
         }
         if( ( num1 != recordObj["acc_no"] && num2 != recordObj["acc_no"] ) ||
             ( "test" !=  recordObj["context"] &&  "test1" !=  recordObj["context"] ) ||
             ( "中文测试" != recordObj["text"] && "中文测试1" != recordObj["text"] ) )
         {
            println( "records: " + queryRecords[i] );
            throw "ErrorFieldValueInRecord1";
         }
      }
      cmd.run( "rm -rf " + FileName );
      println( "test over" ) ;
   }
   catch( e )
   {
      cmd.run( "rm -rf " + FileName );
      throw e;
   }
}

try
{
   initGlobalVar() ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
              "drop collection in the beginning" ) ;
               
   main( db ) ;
   
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
              "drop collection in the beginning" ) ;

}
catch( e )
{
   throw e ;

}
