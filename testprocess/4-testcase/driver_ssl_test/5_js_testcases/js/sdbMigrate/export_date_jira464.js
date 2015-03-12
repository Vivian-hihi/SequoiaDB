/*******************************************************************************
*@Description : export the record that type is date.
*               [SEQUOIADBMAINSTREAM-464]
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   try
   {
      var FILENAME_J = LocalPath + "/" + CSPREFIX + "_option_dateJira464_json.file",
          FILENAME_C = LocalPath + "/" + CSPREFIX + "_option_dateJira464_csv.file",
          expCmd = InstallPath + "/bin/sdbexprt",
          impCmd = InstallPath + "/bin/sdbimprt",
          cmd = new Cmd(),
          NEWCLNAME = CSPREFIX + "_CL_Import_Verify",
          dateRecord = new Array( { "key" : { "$date" : "1990-09-21" } },
                                  { "key" : { "$date" : "2010-12-01" } },
                                  { "key" : { "$date" : "2014-11-09" } },
                                  { "key" : { "$date" : "2015-01-06" } } ) ;

      // create cs
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                             "create collection in the beginning" ) ;
      cmdInsert( cl, dateRecord.length, false, dateRecord ) ;
      println( "success to insert " + dateRecord.length + " records" ) ;
      // create new collection
      commDropCL( db, COMMCSNAME, NEWCLNAME, true, true,
                  "failed to clean new collection" ) ;
      var newCL = commCreateCL( db, COMMCSNAME, NEWCLNAME, -1, true, true,
                                "failed to create new collection" ) ;

      // export json and csv
      var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME_J+
                         " --type json" ;
      cmdToolRun( expCmd, expCmdOption ) ;
      var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME_C+
                         " --type csv --fields '_id,key'" ;
      cmdToolRun( expCmd, expCmdOption ) ;
      println( "success to export data" ) ;
/*******************************************************************************
*@【Test Point1】: test export "date" type data. export type : json
*******************************************************************************/
      var impCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+NEWCLNAME+" --file "+FILENAME_J+
                         " --type json" ;
      try
      {
         cmdToolRun( impCmd, impCmdOption ) ;
         for( var i = 0 ; i < dateRecord.length ; ++i )
         {
            var query1 = cl.find( dateRecord[i] ).toArray()
            var query2 = newCL.find( dateRecord[i] ).toArray()
            var clObj = eval( "(" + query1 + ")" ) ;
            var newCLObj = eval( "(" + query2 + ")" ) ;
            if( JSON.stringify( query1 ) != JSON.stringify( query2 ) ||
                dateRecord[i].key.$date != clObj.key.$date ||
                dateRecord[i].key.$date != newCLObj.key.$date )
            {
               println( "expect date: " + typeof(dateRecord[i].key.$date) ) ;
               println( "actural date: " + typeof(clObj.key.$date) ) ;
               println( "expect record: " + cl.find( dateRecord[i] ) ) ;
               println( "actural record: " + newCL.find( dateRecord[i] ) ) ;
               throw "ErrDateRecordJsonImport" ;
            }
         }
         println( "success to run json import" ) ;
      }
      catch( e )
      {
         throw e ;
      }
      newCL.remove() ;
/*******************************************************************************
*@【Test Point2】: test export "date" type data. export type : csv
*******************************************************************************/
      var impCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+NEWCLNAME+" --file "+FILENAME_C+
                         " --type csv --headerline true --fields '_id oid, key date'" ;
      try
      {
         cmdToolRun( impCmd, impCmdOption ) ;
         for( var i = 0 ; i < dateRecord.length ; ++i )
         {
            var query1 = cl.find( dateRecord[i] ).toArray()
            var query2 = newCL.find( dateRecord[i] ).toArray()
            var clObj = eval( "(" + query1 + ")" ) ;
            var newCLObj = eval( "(" + query2 + ")" ) ;
            if( JSON.stringify( query1 ) != JSON.stringify( query2 ) ||
                dateRecord[i].key.$date != clObj.key.$date ||
                dateRecord[i].key.$date != newCLObj.key.$date )
            {
               println( "expect record: " + cl.find( dateRecord[i] ) ) ;
               println( "actural record: " + newCL.find( dateRecord[i] ) ) ;
               throw "ErrDateRecordCsvImport" ;
            }
         }
         println( "success to run csv import" ) ;
      }
      catch( e )
      {
         throw e ;
      }
      commDropCL( db, COMMCSNAME, NEWCLNAME, false, false,
                  "drop collection in the beginning" ) ;
      cmd.run( "rm -rf " + FILENAME_J ) ;
      cmd.run( "rm -rf " + FILENAME_C ) ;
   }
   catch( e )
   {
      commDropCL( db, COMMCSNAME, NEWCLNAME, true, true,
                  "drop collection in the beginning" ) ;
      cmd.run( "rm -rf " + FILENAME_J ) ;
      cmd.run( "rm -rf " + FILENAME_C ) ;
      throw e ;
   }

}

// Run Main
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
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop collection in the beginning" ) ;
   throw e ;
}
