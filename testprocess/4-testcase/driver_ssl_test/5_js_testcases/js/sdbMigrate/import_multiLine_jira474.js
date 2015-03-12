/*******************************************************************************
*@Description : import one record that across multipile lines.
*               [SEQUOIADBMAINSTREAM-474]
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   try
   {
      var FILENAME = LocalPath + "/" + CSPREFIX + "_multi_jira474.file",
          impCmd = InstallPath + "/bin/sdbimprt",
          nestNum = 10 ;
          recordNum = 100 ;
          cmd = new Cmd() ;

      // create cs
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                             "create collection in the beginning" ) ;
      //cmdInsert( cl, dateRecord.length, false, dateRecord ) ;
      //println( "success to insert " + dateRecord.length + " records" ) ;

      var nest = migNestObject( nestNum ) ;
      cmd.run( "rm -rf " + FILENAME ) ;
      var file = new File( FILENAME ) ;
      var nestRecords = "" ;
      for( var i = 0 ; i < recordNum ; ++i )
      {
         //nestRecords = nestRecords + '\n' + nest ;
         nestRecords = nestRecords + nest ;
      }
      file.write( nestRecords ) ;
      println( "auto generate " + recordNum + " records" ) ;
/*******************************************************************************
*@【Test Point1】: import a record that this record across multipile lines and
*                  default '--linepriority' is true. equal '--linepriority true'
*******************************************************************************/
      var impCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                         " --type json " ;
      try
      {
         cmdToolRun( impCmd, impCmdOption ) ;
         println( "success to import record by user default: --linepriority" ) ;
         var queryNum = cl.find().count() ;
         if( 0 != queryNum )
         {
            println( "error record insert: " +cl.find() ) ;
            throw "ErrRecordInsert" ;
         }
      }
      catch( e )
      {
         println( "failed to import data by use default: --linepriority" + e ) ;
         throw e ;
      }
/*******************************************************************************
*@【Test Point2】: import a record that this record across multipile lines and
*                  specify option: '--linepriority false'
*******************************************************************************/
      var impCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                         " --type json --linepriority false" ;
      try
      {
         cmdToolRun( impCmd, impCmdOption ) ;
         println( "success to import data by using '--linepriority false'" ) ;
         var queryNum = cl.find().count() ;
         if( recordNum != queryNum )
         {
            println( "expect number: " + recordNum + ", actual number: " + queryNum ) ;
            throw "ErrImportNumberRecord" ;
         }
         //var queryCond = '{}, {"nestField_0":1}' ;
         var query = cl.find( {}, {"nestField_0":1} ).toArray() ;
         for( var i = 0 ; i < query.length ; ++i )
         {
            var content = query[i] ;
            if( content != nest )
            {
               println( "expect record content : " + nest ) ;
               println( "actual record content : " + content ) ;
               throw "ErrImportContent" ;
            }
         }
         println( "success to verify import" ) ;
      }
      catch( e )
      {
         println( "failed to import multipile lines, rc = " + e ) ;
         throw e ;
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
