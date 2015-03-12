/*******************************************************************************
*@Description : import data that type is $oid, $binary, $regex. When binary data
*               in import file is wrong, the import tool will core dump.[bug fix]
*               [SEQUOIADBMAINSTREAM-533]
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   try
   {
      var FILENAME = LocalPath + "/" + CSPREFIX + "_csvMig_newType_Jira467.file",
          expCmd = InstallPath + "/bin/sdbexprt",
          impCmd = InstallPath + "/bin/sdbimprt",
          recordNum = 40,
          cmd = new Cmd(),
          NEWCLNAME = CSPREFIX + "_CL_Import_Verify";
          record = new Array( { "key_oid" : {"$oid": "123abcd00ef12358902300ef"},
                                "key_binary" : {"$binary":"aGVsbG8gd29ybGQ=",
                                                 "$type" : 1 },  // error binary
                                "key_regex" : { "$regex" : "^张",
                                                "$options" : "i" }}) ;
      // create collection and insert record
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                             "create collection in the beginning" ) ;
      cmdInsert( cl, recordNum, false, record ) ;
      println( "success to insert data" ) ;
      //println( cl.find() );
/*******************************************************************************
*@【Test Point1】: export complete binary and regex.
*                  Arguement: --includebinary true --includeregex true
*******************************************************************************/
      var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                         " --type csv --includebinary true --includeregex true"+
                         " --fields 'key_oid, key_binary, key_regex'" ;
      try
      {
         cmdToolRun( expCmd, expCmdOption ) ;
         println( "success to export data" ) ;
      }
      catch( e )
      {
         println( "failed to export csv format data " ) ;
      }
/*******************************************************************************
*@【Test Point2】: import complete new type: $oid/$binary/$regex
*******************************************************************************/
      // create new collection
      commDropCL( db, COMMCSNAME, NEWCLNAME, true, true,
                  "failed to clean new collection" ) ;
      var newCL = commCreateCL( db, COMMCSNAME, NEWCLNAME, -1, true, true,
                                "failed to create new collection" ) ;
      var impCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+NEWCLNAME+" --file "+FILENAME+
                         " --type csv --headerline true -n 1"+
                         " --fields 'key_oid oid,key_binary binary,key_regex regex'" ;
      try
      {
         // import data
         cmdToolRun( impCmd, impCmdOption ) ;
         println( "success to import data" ) ;
         // verify import data. the wrong binary data will not import to DB
         if( recordNum != newCL.count() )
         {
            println( JSON.stringify( record[0] ) ) ;
            println( "expect query number: " + recordNum +
                     ", actual query number: " + newCL.find().count() ) ;
            throw "ErrorImportNewTypeData" ;
         }
      }
      catch( e )
      {
         println( "failed to import data" ) ;
         throw e ;
      }
      cmd.run( "rm -rf " + FILENAME ) ;
      commDropCL( db, COMMCSNAME, NEWCLNAME, false, false,
                  "failed to clean new collection" ) ;
   }
   catch( e )
   {
      cmd.run( "rm -rf " + FILENAME ) ;
      commDropCL( db, COMMCSNAME, NEWCLNAME, true, true,
                  "failed to clean new collection" ) ;
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
