/******************************************************************************
*@Description : test export and import's parameters: --collection
*               hash split and percent split
*@Command :
*  export: ./sdblobtool --hostname XXX --svcname XXX --operation export
*                       --collection XXX --file  XXX/XXX
*  import: ./sdblobtool --hostname XXX --svcname XXX --operation import
*                       --collection XXX --file  XXX/XXX
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_export_cl.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;

   var exportCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation export" +
                   " --collection " + expFullCL + " --file " + exportFile ;
   var importCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation import" +
                   " --collection " + impFullCL + " --file " + exportFile ;
   println( "all test export command : " ) ;
   println( "                          [" + exportCmd + "]") ;
   println( "all test import command : " ) ;
   println( "                          [" + importCmd + "]" ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is hash split in export.
*******************************************************************************/
   var expSplit = "hash" ;
   println( "==>>[TP1]hash split in export collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is hash percent split in export.
*******************************************************************************/
   var expSplit = "hash_percent" ;
   println( "==>>[TP2]hash percent split in export collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is hash split in import.
*******************************************************************************/
   var expSplit = null ;
   var impSplit = "hash" ;
   println( "==>>[TP3]hash split in import collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit, impSplit ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is hash percent split in import.
*******************************************************************************/
   var expSplit = null ;
   var impSplit = "hash_percent" ;
   println( "==>>[TP4]hash percent split in import collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit, impSplit ) ;
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
