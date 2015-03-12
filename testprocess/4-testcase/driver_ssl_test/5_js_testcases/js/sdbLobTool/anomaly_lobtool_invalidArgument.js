/******************************************************************************
*@Description : anomaly test invalid argument
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/
/*
function toolCmd( toolArg )
{
   var toolCmd = InstallPath + "/bin/sdblobtool --hostname " + toolArg[0] +
                 " --svcname " + toolArg[1] + " --usrname " + toolArg[2] +
                 " --passwd " + toolArg[3] + " --operation " + toolArg[4] +
                 " --collection " + toolArg[5] + " --file " + toolArg[6] +
                 " --ignorefe " + toolArg[7] + " --dsthost " + toolArg[8] +
                 " --dstservice " + toolArg[9] + " --dstusrname " +
                   toolArg[10] + " --dstpasswd " + toolArg[11] +
                 " --dstcollection " + toolArg[12] ;
   return toolCmd ;

}
*/

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_anomaly_invalidArg.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;
   // 13 parameters and common command
   var toolArg = new Array( 13 ) ;
/*
   var toolCmd = InstallPath + "/bin/sdblobtool --hostname " + toolArg[0] +
                 " --svcname " + toolArg[1] + " --usrname " + toolArg[2] +
                 " --passwd " + toolArg[3] + " --operation " + toolArg[4] +
                 " --collection " + toolArg[5] + " --file " + toolArg[6] +
                 " --ignorefe " + toolArg[7] + " --dsthost " + toolArg[8] +
                 " --dstservice " + toolArg[9] + " --dstusername " +
                   toolArg[10] + " --dstpasswd " + toolArg[11] +
                 " --dstcollection " + toolArg[12] ;
*/
   var opera = new Array( "export", "import", "migration" ) ;
/******************************************************************************
*@Test Description:  lob tool. inspect the unknown argument : --KKERROR
*******************************************************************************/
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
   for( var i = 0 ; i < opera.length ; ++i )
   {
      println( "***<run mode is : " + opera[i] + ">***" ) ;
      toolArg[4] = opera[i] ;
      if( "export" == opera[i] )
         Cmd = toolCmd( toolArg ) + " --KKERROR errortest" ;
      else if( "import" == opera[i] )
      {
         toolArg[4] = "export" ;        // --operation
         exportCmd = toolCmd( toolArg ) ;
         println( "[command : " + exportCmd + "]" ) ;
         testToolAnomalyPara( db, lobNum, exportCmd, exportFile, newCLNAME,
                              null, null, null, "export" ) ;
         toolArg[4] = opera[i] ;
         toolArg[5] = impFullCL ;
         Cmd = toolCmd( toolArg ) + " --KKERROR errortest" ;
      }
      else
      {
         cmd.run( "rm -rf " + exportFile ) ;
         toolArg[5] = expFullCL ;
         toolArg[8] = COORDHOSTNAME ;   // --hostname
         toolArg[9] = COORDSVCNAME ;    // --svcname
         toolArg[12] = impFullCL ;
         Cmd = toolCmd( toolArg ) + " --KKERROR errortest" ;
      }
      println( "==>>" + opera[i] + " command : " + Cmd ) ;
      try
      {
         testToolAnomalyPara( db, lobNum, Cmd, exportFile, newCLNAME,
                              null, null, null, opera[i] ) ;
         throw "test lob tool error" ;
      }
      catch( e )
      {
         if( 127 != e )
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
