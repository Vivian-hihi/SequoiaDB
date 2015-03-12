/******************************************************************************
*@Description : test migaration's parameters: --hostname/--svcname
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
   var newCLNAME = COMMCLNAME + "_dst" ;
   var dstMigFullCL = COMMCSNAME + "." + newCLNAME ;
/******************************************************************************
*@Test Description:  export and import use default parameters
*  migration: ./sdblobtool --operation migration --collection XXX --file XXX/XXX
*                          --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool " +
                   "--operation migration --collection " + srcMigFullCL +
                   " --file " + migFile + " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP1] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --hostname localhost
*  migration: ./sdblobtool --operation migration --collection XXX --file XXX/XXX
*                          --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname localhost " +
                   "--operation migration --collection " + srcMigFullCL +
                   " --file " + migFile + " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP2] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --hostname XXX
*  migration: ./sdblobtool --hostname XXX --operation migration --collection XXX
*                          --file XXX/XXX --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --operation migration --collection " + srcMigFullCL +
                   " --file " + migFile + " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP3] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --svcname XXX
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation migration " +
                   "--collection " + srcMigFullCL +
                   " --file " + migFile + " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP4] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --svcname XXX
*  migration: ./sdblobtool --hostname localhost --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname localhost" +
                   " --svcname " + COORDSVCNAME + " --operation migration " +
                   "--collection " + srcMigFullCL +
                   " --file " + migFile + " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP5] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --svcname XXX
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dstcollection XXX
*******************************************************************************/
   var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                " --svcname " + COORDSVCNAME + " --operation migration " +
                "--collection " + srcMigFullCL + " --dsthost " +
                COORDHOSTNAME + " --dstservice " + COORDSVCNAME +
                " --file " + migFile + " --dstcollection " + dstMigFullCL ;
   println( "==>>[TP6] test migration command : " + migCmd ) ;
   testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
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
