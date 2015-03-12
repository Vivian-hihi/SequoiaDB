/******************************************************************************
*@Description : test migaration's parameters: --dstHost/--dstService
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/
function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_mig_dsthostsvcname.file" ;
   var migFile = LocalPath + "/" + filename ;
   var srcMigFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_dst" ;
   var dstMigFullCL = COMMCSNAME + "." + newCLNAME ;
/******************************************************************************
*@Test Description:  export and import use default parameters
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation migration " +
                   "--collection " + srcMigFullCL +
                   " --file " + migFile + " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP1] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --dsthost localhost
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dsthost localhost
*                          --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation migration " +
                   "--collection " + srcMigFullCL + " --file " + migFile +
                   " --dsthost localhost --dstcollection " + dstMigFullCL ;
      println( "==>>[TP2] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --dsthost XXX
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dsthost XXX
*                          --dstcollection XXX
*******************************************************************************/
   if( 11810 == COORDSVCNAME )
   {
      var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation migration " +
                   "--collection " + srcMigFullCL + " --file " + migFile +
                   " --dsthost " + COORDHOSTNAME + " --dstcollection " +
                   dstMigFullCL ;
      println( "==>>[TP3] test migration command : " + migCmd ) ;
      testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
   }
/******************************************************************************
*@Test Description:  migrate data by using parameter: --svcname XXX
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dsthost XXX
*                          --dstservice XXX --dstcollection XXX
*******************************************************************************/
   var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                " --svcname " + COORDSVCNAME + " --operation migration " +
                "--collection " + srcMigFullCL + " --file " + migFile +
                " --dsthost " + COORDHOSTNAME + " --dstservice " + COORDSVCNAME
                + " --dstcollection " + dstMigFullCL ;
   println( "==>>[TP4] test migration command : " + migCmd ) ;
   testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
/******************************************************************************
*@Test Description:  migrate data by using parameter: --svcname XXX
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dsthost XXX
*                          --dstservice XXX --dstcollection XXX
*******************************************************************************/
   var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                " --svcname " + COORDSVCNAME + " --operation migration " +
                "--collection " + srcMigFullCL + " --file " + migFile +
                " --dsthost localhost --dstservice " + COORDSVCNAME +
                " --dstcollection " + dstMigFullCL ;
   println( "==>>[TP5] test migration command : " + migCmd ) ;
   testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
/******************************************************************************
*@Test Description:  migrate localhost data to remote host data in same Sdb
*  migration: ./sdblobtool --hostname XXX --svcname XXX --operation migration
*                          --collection XXX --file XXX/XXX --dsthost [remote]
*                          --dstservice XXX --dstcollection XXX
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
      var migCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation migration " +
                   "--collection " + srcMigFullCL + " --file " + migFile +
                   " --dsthost " + remoteHost + " --dstservice " + remoteSvc +
                   " --dstcollection " + dstMigFullCL ;
      println( "==>>[TP6] test migration command : " + migCmd ) ;
      if( null != remoteHost )
      {
         testMigrationPara( db, lobNum, migCmd, migFile, newCLNAME ) ;
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
