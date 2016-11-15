/*******************************************************************************
*@Description : import data that the document auto add one field.
*               such:{a:1}, after insert become:{a:1, field1:null}
*               [SEQUOIADBMAINSTREAM-608]
*@Modify list :
*               2015-3-3  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   var FileName = LocalPath + "/" + CHANGEDPREFIX + "_importAutoAddOneField_jira608.file" ;
   var impCmd = InstallPath + "/bin/sdbimprt" ;
   var cmd = new Cmd();
   cmd.run( "rm -rf " + FileName );
   var fileObj = new File( FileName ) ;
   fileObj.write("13434347996,test,中文测试\n13434347997,test1,中文测试1") ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                          "create collection in the beginning" ) ;
   var impCmdOption = "--hostname " + COORDHOSTNAME + " --svcname " + COORDSVCNAME +
                      " -c " + COMMCSNAME + " -l " + COMMCLNAME + " --file " +
                      FileName + " --type csv --fields \"acc_no,context,text\" -n 1 ";
   if( false == commIsStandalone( db ) )
      db.setSessionAttr( {"PreferedInstance":"M"} ) ;
   try
   {
      cmdToolRun( impCmd, impCmdOption );
      println( "success to insert data" );
      if( 2 != cl.count() )
      {
         println( "expect records: 2, actural records: " + cl.count() );
         throw "ErrorRecordsNumber";
      }
      var queryRecords = cl.find().toArray();
      for( var i = 0 ; i < queryRecords.length ; ++i )
      {
         var recordObj = JSON.parse( queryRecords[i] );
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
         if( 0 == i )
         {
            if( 13434347996 != recordObj["acc_no"] ||
                "test" !=  recordObj["context"] || "中文测试" != recordObj["text"] )
            {
               println( "records: " + queryRecords[i] );
               throw "ErrorFieldValueInRecord1";
            }
         }
         else
         {
            if( 13434347997 != recordObj["acc_no"] ||
                "test1" !=  recordObj["context"] || "中文测试1" != recordObj["text"] )
            {
               println( "records: " + queryRecords[i] );
               throw "ErrorFieldValueInRecord2";
            }
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
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
   //            "drop collection in the beginning" ) ;

}
catch( e )
{
   //commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
   //            "drop collection in the beginning" ) ;
   throw e ;

}
