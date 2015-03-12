/******************************************************************************
*@Description : anomaly test, --hostname/--svcname/--operatione/--collection/
*               --file/--dstcollection/--dsthost/--dstservice parameters have
*               no value
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_no_option_value.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;
   // 13 parameters and common command
   var toolArg = new Array( 13 ) ;

   var opera = new Array( "export", "import", "migration" ) ;
   // init first
   toolArg[0] = COORDHOSTNAME ;   // --hostname
   toolArg[1] = COORDSVCNAME ;    // --svcname
   toolArg[2] = null ;            // --usrname
   toolArg[3] = null ;            // --passwd
   toolArg[4] = "export" ;        // --operation
   toolArg[5] = expFullCL ;       // --collection
   toolArg[6] = exportFile ;      // --file
   toolArg[7] = false ;           // --ignorefe
   toolArg[8] = COORDHOSTNAME ;   // --dsthost
   toolArg[9] = COORDSVCNAME ;    // --dstservice
   toolArg[10] = null ;           // --dstusrname
   toolArg[11] = null ;           // --dstpasswd
   toolArg[12] = null ;           // --dstcollection
/******************************************************************************
*@Test Description:  lob tool. inspect the no argument value : --hostname
*******************************************************************************/
   toolArg[0] = "" ;              // --hostname[TP]
   toolArg[4] = "export" ;        // --operation
   toolArg[5] = expFullCL ;       // --collection
   toolArg[6] = exportFile ;      // --file
   toolArg[7] = null ;            // --ignorefe
   toolArg[12] = impFullCL ;      // dstcollection
   for( var i = 0 ; i < opera.length ; ++i )
   {
      println( "***<run mode is : " + opera[i] + ">***" ) ;
      cmd.run( "rm -rf " + exportFile ) ;
      if( "import" == opera[i] )
      {
         toolArg[0] = COORDHOSTNAME ;
         toolArg[1] = COORDSVCNAME ;
         toolArg[4] = "export" ;
         exportCmd = toolCmd( toolArg ) ;
         println( "[command : " + exportCmd + "]" ) ;
         testToolAnomalyPara( db, lobNum, exportCmd, exportFile, newCLNAME,
                              null, null, null, "export" ) ;
         toolArg[0] = "" ;
         exportCmd = toolCmd( toolArg ) ;
         toolArg[5] = impFullCL ;       // --collection
      }
      toolArg[4] = opera[i] ;
      Cmd = toolCmd( toolArg ) ;
      println( "==>>" + opera[i] + " command : " + Cmd ) ;
      try
      {
         testToolAnomalyPara( db, lobNum, Cmd, exportFile, newCLNAME,
                              null, null, null, opera[i] ) ;
         throw "test lob tool error" ;
      }
      catch( e )
      {
         if( 135 != e )
         {
            println( "failed to test anormaly, rc = " + e ) ;
            throw e ;
         }
      }
   }
}

// Running
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clean collection in the beginning" ) ;
   main( db ) ;
   db.close() ;
}
catch( e )
{
   db.close() ;
   throw e ;
}
