/******************************************************************************
*@Description : test import and migaration data to same collection
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_same_collection.file" ;
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
   toolArg[12] = expFullCL ;           // --dstcollection
   for( var i = 1 ; i < opera.length ; ++i )
   {
      try
      {
         println( "***<run mode is : " + opera[i] + ">***" ) ;
         toolArg[4] = opera[i] ;
         if( "import" == opera[i] )
         {
            toolArg[0] = COORDHOSTNAME ;
            toolArg[1] = COORDSVCNAME ;
            toolArg[4] = "export" ;
            exportCmd = toolCmd( toolArg ) ;
            println( "[command : " + exportCmd + "]" ) ;
            testToolAnomalyPara( db, lobNum, exportCmd, exportFile, COMMCLNAME,
                                 null, null, null, "export" ) ;
            toolArg[4] = opera[i] ;
            toolArg[5] = expFullCL ;
            Cmd = toolCmd( toolArg ) ;
            testToolAnomalyPara( db, lobNum, Cmd, exportFile, COMMCLNAME,
                                 null, null, null, opera[i] ) ;
         }
         else
         {
            cmd.run( "rm -rf " + exportFile ) ;
            toolArg[5] = expFullCL ;
            toolArg[8] = COORDHOSTNAME ;
            toolArg[9] = COORDSVCNAME ;
            Cmd = toolCmd( toolArg ) ;
            testToolAnomalyPara( db, lobNum, Cmd, exportFile, -1,
                                 null, null, null, opera[i] ) ;
         }
         println( "==>>" + opera[i] + " command : " + Cmd ) ;
         println( opera[i] + " data in same collection success.") ;
      }
      catch( e )
      {
         if( 8 != e )
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
