/******************************************************************************
*@Description : test export and import's parameters: --hostname/--svcname
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/
function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX+ "_export.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;
/******************************************************************************
*@Test Description:  export and import use : --hostname/--svcname[default value]
*  export: ./sdblobtool --operation export --collection XXX --file XXX/XXX
*  import: ./sdblobtool --operation import --collection XXX --file XXX/XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var exportCmd = InstallPath + "/bin/sdblobtool " +
                      " --operation export --collection " + expFullCL +
                      " --file " + exportFile ;
      var importCmd = InstallPath + "/bin/sdblobtool " +
                      " --operation import --collection " + impFullCL +
                      " --file " + exportFile ;
      println( "==>>[TP1]test export command : " + exportCmd ) ;
      println( "==>>[TP1]test import command : " + importCmd ) ;
      testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                            newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  export and import use : --hostname localhost/
*                                            --svcname[default value]
*  export: ./sdblobtool --hostname localhost --operation export --collection XXX
*                       --file XXX/XXX
*  import: ./sdblobtool --hostname localhost --operation import --collection XXX
*                       --file XXX/XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var exportCmd = InstallPath + "/bin/sdblobtool --hostname localhost" +
                      " --operation export --collection " + expFullCL +
                      " --file " + exportFile ;
      var importCmd = InstallPath + "/bin/sdblobtool --hostname localhost" +
                      " --operation import --collection " + impFullCL +
                      " --file " + exportFile ;
      println( "==>>[TP2]test export command : " + exportCmd ) ;
      println( "==>>[TP2]test import command : " + importCmd ) ;
      testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                            newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  export and import use : --hostname XXX/
*                                            --svcname[default value]
*  export: ./sdblobtool --hostname XXX --operation export --collection XXX
*                       --file XXX/XXX
*  import: ./sdblobtool --hostname XXX --operation import --collection XXX
*                       --file XXX/XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var exportCmd = InstallPath + "/bin/sdblobtool --hostname " +
                      COORDHOSTNAME + " --operation export --collection " +
                      expFullCL + " --file " + exportFile ;
      var importCmd = InstallPath + "/bin/sdblobtool --hostname " +
                      COORDHOSTNAME + " --operation import --collection " +
                      impFullCL + " --file " + exportFile ;
      println( "==>>[TP3]test export command : " + exportCmd ) ;
      println( "==>>[TP3]test import command : " + importCmd ) ;
      testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                            newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  export and import use : --hostname localhost/
*                                            --svcname XXX
*  export: ./sdblobtool --hostname localhost --svcname XXX --operation export
*                       --collection XXX --file XXX/XXX
*  import: ./sdblobtool --hostname localhost --svcname XXX --operation import
*                       --collection XXX --file XXX/XXX
*******************************************************************************/
   var exportCmd = InstallPath + "/bin/sdblobtool --hostname localhost" +
                   " --svcname " + COORDSVCNAME + " --operation export" +
                   " --collection " + expFullCL + " --file " + exportFile ;
   var importCmd = InstallPath + "/bin/sdblobtool --hostname localhost" +
                   " --svcname " + COORDSVCNAME + " --operation import" +
                   " --collection " + impFullCL + " --file " + exportFile ;
   println( "==>>[TP4]test export command : " + exportCmd ) ;
   println( "==>>[TP4]test import command : " + importCmd ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME ) ;
/******************************************************************************
*@Test Description:  export and import use : --hostname XXX/ --svcname XXX
*  export: ./sdblobtool --hostname XXX --svcname XXX --operation export
*                       --collection XXX --file XXX/XXX
*  import: ./sdblobtool --hostname XXX --svcname XXX --operation import
*                       --collection XXX --file XXX/XXX
*******************************************************************************/
   var exportCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation export" +
                   " --collection " + expFullCL + " --file " + exportFile ;
   var importCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation import" +
                   " --collection " + impFullCL + " --file " + exportFile ;
   println( "==>>[TP5]test export command : " + exportCmd ) ;
   println( "==>>[TP5]test import command : " + importCmd ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME ) ;
/******************************************************************************
*@Test Description:  export and import use : --hostname XXX/ --svcname XXX
*  export: ./sdblobtool --hostname XXX --svcname XXX --operation export
*                       --collection XXX --file XXX/XXX
*  import: ./sdblobtool --hostname XXX --svcname XXX --operation import
*                       --collection XXX --file XXX/XXX
*******************************************************************************/
   if( false == commIsStandalone( db ) )
   {
      var remoteHost = null ;
      var remoteSvc = null ;
      var local = cmd.run( "hostname" ).split( "\n" ) ;
      var localHost = local[0] ;
      var group = commGetGroups( db ) ;
      for( var i = 0 ; i < group.length ; ++i )
      {
         for( var j = 1 ; j < group[i][0].length ; ++j )
         {
            if( localHost != group[i][j].HostName )
            {
               remoteHost = group[i][j].HostName ;
               remoteSvc = group[i][j].svcname ;
               break ;
            }
         }
      }
      var exportCmd = InstallPath + "/bin/sdblobtool --hostname " + remoteHost +
                      " --svcname" + remoteSvc + " --operation export" +
                      " --collection " + expFullCL + " --file " + exportFile ;
      var importCmd = InstallPath + "/bin/sdblobtool --hostname " + remoteHost +
                      " --svcname" + remoteSvc + " --operation import" +
                      " --collection " + impFullCL + " --file " + exportFile ;
      println( "==>>[TP6]test export command : " + exportCmd ) ;
      println( "==>>[TP6]test import command : " + importCmd ) ;
      if( null != remoteHost )
      {
         testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                               newCLNAME ) ;
      }
      else
         println( "this group only have one host" ) ;
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
