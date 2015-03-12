/*******************************************************************************
*@Description : export the data, and the field delimter, string delimter and
                record delimter are equal with each other.
*               [SEQUOIADBMAINSTREAM-468]
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   try
   {
      var FILENAME = LocalPath + "/" + CSPREFIX +
                     "_exportCSVSameDelimiter_Jira468.file",
          expCmd = InstallPath + "/bin/sdbexprt",
          delimiter = new Array( '\'"\'', '\',\'', '\'0x0A\''),
          recordNum = 50,
          cmd = new Cmd() ;
      // create collection
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                             "create collection in the beginning" ) ;
      cmdInsert( cl, recordNum ) ;
      println( "success to insert " + recordNum + " records" ) ;
/*******************************************************************************
*@【Test Point1】: export data that the string delimiter, field delimiter and
*                  record delimiter are equal with each other
*******************************************************************************/
      for( var i = 0 ; i < delimiter.length ; ++i )
      {
         // the three of delimiter are equale with each other
         var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                            " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                            " --type csv --fields 'key' -a "+delimiter[i]+
                            " --delfield "+delimiter[i]+" --delrecord "+delimiter[i] ;
         try
         {
            cmdToolRun( expCmd, expCmdOption ) ;
            println( "success to export json format data by assign not exist CL" ) ;
            throw "ErrorDelimiterExport" ;
         }
         catch( e )
         {
            if( 127 != e )
            {
               throw e ;
            }
         }
         // field delimiter is equale with record delimiter
         var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                            " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                            " --type csv --fields 'key' -a '0x24'"+
                            " --delfield "+delimiter[i]+" --delrecord "+delimiter[i] ;
         try
         {
            cmdToolRun( expCmd, expCmdOption ) ;
            println( "success to export json format data by assign not exist CL" ) ;
            throw "ErrorDelimiterExport" ;
         }
         catch( e )
         {
            if( 127 != e )
            {
               throw e ;
            }
         }
         // string delimiter is equale with record delimiter
         var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                            " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                            " --type csv --fields 'key' -a "+delimiter[i]+
                            " --delfield '0x24' --delrecord "+delimiter[i] ;
         try
         {
            cmdToolRun( expCmd, expCmdOption ) ;
            println( "success to export json format data by assign not exist CL" ) ;
            throw "ErrorDelimiterExport" ;
         }
         catch( e )
         {
            if( 127 != e )
            {
               throw e ;
            }
         }
         // field delimiter is equale with string delimiter
         var expCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                            " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                            " --type csv --fields 'key' -a "+delimiter[i]+
                            " --delfield "+delimiter[i]+" --delrecord '0x24'" ;
         try
         {
            cmdToolRun( expCmd, expCmdOption ) ;
            println( "success to export json format data by assign not exist CL" ) ;
            throw "ErrorDelimiterExport" ;
         }
         catch( e )
         {
            if( 127 != e )
               throw e ;
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
