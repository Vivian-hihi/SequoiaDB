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
*                    collection split is hash split in export and import.
*******************************************************************************/
   var expSplit = "hash" ;
   var impSplit = "hash" ;
   println( "==>>[TP5]hash split in import collection and export collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit, impSplit ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is hash percent split in export and import.
*******************************************************************************/
   var expSplit = "hash_percent" ;
   var impSplit = "hash_percent" ;
   println( "==>>[TP6]hash percent split in import collection " +
                      "and export collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit, impSplit ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is that hash split in export and hash percent
*                    split in import.
*******************************************************************************/
   var expSplit = "hash" ;
   var impSplit = "hash_percent" ;
   println( "==>>[TP7]hash split in export collection " +
                      "and hash percent split in import collection" ) ;
   testExportImportPara( db, lobNum, exportCmd, exportFile, importCmd,
                         newCLNAME, expSplit, impSplit ) ;
/******************************************************************************
*@test description:  export and import use parameter: --collection.
*                    collection split is that hash percent split in export and
*                    hash split in import.
*******************************************************************************/
   var expSplit = "hash_percent" ;
   var impSplit = "hash" ;
   println( "==>>[TP8]hash percent split in export collection " +
                      "and hash split in import collection" ) ;
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
