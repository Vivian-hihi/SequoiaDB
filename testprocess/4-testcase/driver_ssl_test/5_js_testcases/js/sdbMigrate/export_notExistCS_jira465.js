/*******************************************************************************
*@Description : export data and collection space not exist
*               [SEQUOIADBMAINSTREAM-465]
*@Modify list :
*               2015-1-16  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   try
   {
      var FILENAME = LocalPath + "/" + CSPREFIX + "_notExistCS_jira465.file",
          expCmd = InstallPath + "/bin/sdbexprt",
          recordNum = 20,
          cmd = new Cmd() ;

      // create cs
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                             "create collection in the beginning" ) ;
      cmdInsert( cl, recordNum ) ;
      println( "success to insert " + recordNum + " records" ) ;
/*******************************************************************************
*@【Test Point1】: export json data when the collection space not exist
*******************************************************************************/
      var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c NotExistCS -l "+COMMCLNAME+" --file "+FILENAME+
                         " --type json" ;
      try
      {
         cmdToolRun( expCmd, expCmdOption ) ;
         println( "success to export json format data by assign not exist CS" ) ;
         throw "ErrorJsonRun" ;
      }
      catch( e )
      {
         if( 8 != e )
         {
            println( "failed to export json data, rc : " + e ) ;
            throw e ;
         }
         else
         {
            println( "test export json format data over" ) ;
         }
      }
/*******************************************************************************
*@【Test Point2】: export csv data when the collection not exist
*******************************************************************************/
      var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c NotExistCS -l "+COMMCLNAME+" --file "+FILENAME+
                         " --type csv --fields '_id,key'" ;
      try
      {
         cmdToolRun( expCmd, expCmdOption ) ;
         println( "success to export csv format data by assign not exist CS" ) ;
         throw "ErrorCsvRun" ;
      }
      catch( e )
      {
         if( 8 != e )
         {
            println( "failed to export csv data, rc : " + e ) ;
            throw e ;
         }
         else
         {
            println( "test export csv format data over" ) ;
         }
      }
      cmd.run( "rm -rf " + FILENAME ) ;
   }
   catch( e )
   {
      cmd.run( "rm -rf " + FILENAME ) ;
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
