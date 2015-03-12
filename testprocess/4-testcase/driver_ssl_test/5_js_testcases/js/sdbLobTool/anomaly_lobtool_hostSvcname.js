/******************************************************************************
*@Description : anomaly test, --hostname/--svcname
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_anomaly_hostsvcname.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;
   // 13 parameters and common command
   var toolArg = new Array( 13 ) ;

   var opera = new Array( "export", "import", "migration" ) ;
/******************************************************************************
*@Test Description:  lob tool. inspect the error argument value :
*                    --hostname/--svcname
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
   var errHostSvc = new Array( "172.168.20.43", "90000" ) ;
   for( var j = 0 ; j < errHostSvc.length ; ++j )
   {
      if( 0 == j )
         toolArg[0] = errHostSvc[0] ;
      else
         toolArg[1] = errHostSvc[1] ;
      for( var i = 0 ; i < opera.length ; ++i )
      {
         println( "***<run mode is : " + opera[i] + ">***" ) ;
         toolArg[4] = opera[i] ;
         if( "export" == opera[i] )
            Cmd = toolCmd( toolArg ) ;
         else if( "import" == opera[i] )
         {
            toolArg[0] = COORDHOSTNAME ;
            toolArg[1] = COORDSVCNAME ;
            toolArg[4] = "export" ;
            exportCmd = toolCmd( toolArg ) ;
            println( "[command : " + exportCmd + "]" ) ;
            testToolAnomalyPara( db, lobNum, exportCmd, exportFile, newCLNAME,
                                 null, null, null, "export" ) ;
            if( 0 == j )
               toolArg[0] = errHostSvc[0] ;
            else
               toolArg[1] = errHostSvc[1] ;
            toolArg[4] = opera[i] ;
            toolArg[5] = impFullCL ;
            Cmd = toolCmd( toolArg ) ;
         }
         else
         {
            cmd.run( "rm -rf " + exportFile ) ;
            toolArg[5] = expFullCL ;
            toolArg[8] = COORDHOSTNAME ;
            toolArg[9] = COORDSVCNAME ;
            toolArg[12] = impFullCL ;
            Cmd = toolCmd( toolArg ) ;
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
            if( 134 != e && 8 != e )
            {
               println( "failed to test anormaly, rc = " + e ) ;
               throw e ;
            }
         }
      }
   }
/******************************************************************************
*@Test Description:  lob tool. inspect the error argument value:
                     --dsthost/--dstservice
*******************************************************************************/
   var errHostSvc = new Array( "172.168.20.43", "90000" ) ;
   for( var j = 0 ; j < errHostSvc.length ; ++j )
   {
      toolArg[4] = "migration" ;
      toolArg[5] = expFullCL ;
      toolArg[8] = COORDHOSTNAME ;
      toolArg[9] = COORDSVCNAME ;
      toolArg[12] = impFullCL ;
      if( 0 == j )
         toolArg[8] = errHostSvc[0] ;
      else
         toolArg[9] = errHostSvc[1] ;
      cmd.run( "rm -rf " + exportFile ) ;
      Cmd = toolCmd( toolArg ) ;

      println( "==>>erorr destnation migration command : " + Cmd ) ;
      try
      {
         testToolAnomalyPara( db, lobNum, Cmd, exportFile, newCLNAME,
                              null, null, null, opera[i] ) ;
         throw "test lob tool error" ;
      }
      catch( e )
      {
         if( 134 != e && 8 != e )
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
