/******************************************************************************
*@Description : anomaly test. do range split, then export/import/migration lob.
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   try
   {
      db.dropUsr( user, passwd ) ;
   }
   catch( e )
   {
      if( -179 != e && -6 != e )
      {
         println( "failed to drop user, rc = " + e ) ;
         throw e ;
      }
   }
   db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_no_rangeCL.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;
   // 13 parameters and common command
   var toolArg = new Array( 13 ) ;
   var opera = new Array( "export", "import", "migration" ) ;
/******************************************************************************
*@Test Description:  lob tool. inspect the error argument value :
*                    --usrname/--passwd
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
   var srcSplit = new Array( "range", "range_percent" ) ;
   var dstSplit = new Array( "range", "range_percent" ) ;
   for( var m = 0 ; m < srcSplit.length ; ++m )
   {
      for( var n = 0 ; n < dstSplit.length ; ++n )
      {
         for( var i = 0 ; i < opera.length ; ++i )
         {
            println( "***<source split : " + srcSplit[m] +
                     "  destnation split : " + dstSplit[n] + ">***" ) ;
            println( "***<run mode is : " + opera[i] + ">***" ) ;
            toolArg[4] = opera[i] ;
            if( "export" == opera[i] )
               Cmd = toolCmd( toolArg ) ;
            else if( "import" == opera[i] )
            {
               toolArg[2] = user ;
               toolArg[3] = passwd ;
               toolArg[4] = "export" ;
               exportCmd = toolCmd( toolArg ) ;
               println( "[command : " + exportCmd + "]" ) ;
               testToolAnomalyPara( db, lobNum, exportCmd, exportFile, newCLNAME,
                                    null, null, null, "export" ) ;
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
                                    srcSplit[m], dstSplit[n], null, opera[i] ) ;
            }
            catch( e )
            {
               if( -6 != e && 127 != e )
               {
                  println( "failed to test anormaly, rc = " + e ) ;
                  throw e ;
               }
            }
         }
      }
   }
}

// Running
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clean collection in the beginning" ) ;
   if( false != commIsStandalone( db ) )
      throw "standalone" ;
   main( db ) ;
   db.close() ;
}
catch( e )
{
   db.close() ;
   if( "standalone" == e )
   {
      println( "sequoiaDB don't have create user and " +
               "password function in standalone" ) ;
   }
   else
      throw e ;
}
