/******************************************************************************
*@Description : test migration's parameters: --collection
*               hash split and percent split
*@Command :
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dsthost XXX
*                          --dstservice XXX --dstcollection XXX
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_mig_hostsvcname.file" ;
   var migFile = LocalPath + "/" + filename ;
   var srcMigFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var migCLNAME = COMMCLNAME + "_dst" ;
   var dstMigFullCL = COMMCSNAME + "." + migCLNAME ;
   var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                " --svcname " + COORDSVCNAME + " --operation migration " +
                "--collection " + srcMigFullCL + " --file " + migFile +
                " --dsthost " + COORDHOSTNAME + " --dstservice " + COORDSVCNAME +
                " --dstcollection " + dstMigFullCL ;
   println( "all test migration command : " ) ;
   println( "                          [" + migCmd + "]") ;
/******************************************************************************
*@test description:  migration and migration use parameter: --collection.
*                    1. collection was split by hash in source migration
*                       colletion
*                    2. collection was split by hash in destination
*                       migration colletion
*******************************************************************************/
   var srcMigSplit = "hash" ;
   var dstMigSplit = "hash" ;
   println( "==>>[TP5]hash split in migration source collection " +
            "and migration destination collection" ) ;
   testMigrationPara( db, lobNum, migCmd, migFile,
                      migCLNAME, srcMigSplit, dstMigSplit )
/******************************************************************************
*@test description:  migration and migration use parameter: --collection.
*                    1. collection was split by hash percent in source migration
*                       colletion
*                    2. collection was split by hash percent in destination
*                       migration colletion
*******************************************************************************/
   var srcMigSplit = "hash_percent" ;
   var dstMigSplit = "hash_percent" ;
   println( "==>>[TP6]hash percent split in migration source collection " +
                      "and migration destination collection" ) ;
   testMigrationPara( db, lobNum, migCmd, migFile,
                      migCLNAME, srcMigSplit, dstMigSplit )
/******************************************************************************
*@test description:  migration and migration use parameter: --collection.
*                    1. collection was split by hash in source migration
*                       colletion
*                    2. collection was split by hash percent in destination
*                       migration colletion
*******************************************************************************/
   var srcMigSplit = "hash" ;
   var dstMigSplit = "hash_percent" ;
   println( "==>>[TP7]hash split in migration source collection " +
                      "and hash percent split in migration destination collection" ) ;
   testMigrationPara( db, lobNum, migCmd, migFile,
                      migCLNAME, srcMigSplit, dstMigSplit )
/******************************************************************************
*@test description:  migration and migration use parameter: --collection.
*                    1. collection was split by hash percent in source migration
*                       colletion
*                    2. collection was split by hash an destination
*                       migration colletion
*******************************************************************************/
   var srcMigSplit = "hash_percent" ;
   var dstMigSplit = "hash" ;
   println( "==>>[TP8]hash percent split in migration source collection " +
                      "and hash split in migration destination collection" ) ;
   testMigrationPara( db, lobNum, migCmd, migFile,
                      migCLNAME, srcMigSplit, dstMigSplit )
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
