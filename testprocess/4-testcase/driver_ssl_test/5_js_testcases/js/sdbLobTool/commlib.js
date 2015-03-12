/******************************************************************************
*@Description : common function for lob import/export/migration tool
*               the testcase about user/password cannot run in concurrent.
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
******************************************************************************/

var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;
var user = "lobtooltest" ;
var passwd = "lobtooltest" ;
var db = new SecureSdb( hostName, coordPort, user, passwd ) ;
var cmd = new Cmd() ;
var LocalPath = null ;
var InstallPath = null ;

// command for lob tool
function toolCmd( toolArg )
{
   if( true == toolArg[7] )
   {
      var toolCmd =
                 InstallPath + "/bin/sdblobtool --hostname " + toolArg[0] +
                 " --svcname " + toolArg[1] + " --usrname " + toolArg[2] +
                 " --passwd " + toolArg[3] + " --operation " + toolArg[4] +
                 " --collection " + toolArg[5] + " --file " + toolArg[6] +
                 " --ignorefe --dsthost " + toolArg[8] +
                 " --dstservice " + toolArg[9] + " --dstusrname " +
                   toolArg[10] + " --dstpasswd " + toolArg[11] +
                 " --dstcollection " + toolArg[12] ;
   }
   else
   {
      var toolCmd =
                 InstallPath + "/bin/sdblobtool --hostname " + toolArg[0] +
                 " --svcname " + toolArg[1] + " --usrname " + toolArg[2] +
                 " --passwd " + toolArg[3] + " --operation " + toolArg[4] +
                 " --collection " + toolArg[5] + " --file " + toolArg[6] +
                 " --dsthost " + toolArg[8] +
                 " --dstservice " + toolArg[9] + " --dstusrname " +
                   toolArg[10] + " --dstpasswd " + toolArg[11] +
                 " --dstcollection " + toolArg[12] ;

   }
   return toolCmd ;

}

// Get Global Variable
/******************************************************************************
*@Description : when run these testcase in sequoiadb or trunk fold that not
*               installed, get home fold.   <?how to get sequoiadb home fold?>
******************************************************************************/
function toolGetInstallPath( localPath )
{
   try
   {
      var folder = cmd.run( 'ls ' + localPath ).split( '\n' ) ;
      var fcnt = 0 ;
      for( var i = 0 ; i < folder.length ; ++i )
      {
         if( "bin" == folder[i] || "SequoiaDB" == folder[i] ||
             "testcase" == folder[i] )
         {
            fcnt++ ;
         }
      }
      if( 2 <= fcnt )
         InstallPath = localPath ;
      return InstallPath ;
   }
   catch( e )
   {
      println( "failed to get install path in source install, rc = " + rc  ) ;
      throw e ;
   }
}

/******************************************************************************
*@Description : initalize the global variable in the begninning.
******************************************************************************/
function initGlobalVar()
{
   try
   {
      var local = cmd.run( "pwd" ).split( "\n" ) ;
      LocalPath = local[0] ;
      try
      {
         var install = cmd.run( "sed -n '3p'  /etc/default/sequoiadb" ).split( "=" ) ;
         var installPath = install[1].split( "\n" ) ;
         InstallPath = installPath[0] ;
      }
      catch( e )
      {
         if( 2 == e )
            InstallPath = toolGetInstallPath( LocalPath ) ;
         else
            throw "failed to excute : sed -n '3p'  /etc/default/sequoiadb" ;
      }
   }
   catch( e )
   {
      println( "failed to get global variable : cmd/LocalPath/InstallPath" + e ) ;
      throw e ;
   }
}

/******************************************************************************
*@Description : main test API for export and import testcase.
*@Parameter :
*             lobNum: the number of lobs that insert
*             exportCmd: the command of export lob
*             exportFile : the export file
*             importCmd: the command of import lob
*             newCLNAME: the collection name that import lob to the collection
*             expSplit: export lob collection split type
*             impSplit: import lob collection split type
******************************************************************************/
function testExportImportPara( db, lobNum, exportCmd, exportFile,
                               importCmd, newCLNAME, expSplit, impSplit )
{
   if( undefined == expSplit ){ expSplit = null ; }
   if( undefined == impSplit ){ impSplit = null ; }
   try {
      // put lob data
      var fileName = CSPREFIX + "_exportDefault.txt" ;
      var lobNum = 100 ;
      toolPutLobs( db, fileName, lobNum ) ;
      println( ">succes to insert " + lobNum + " records" ) ;
      /*** hash split ***/
      if( "hash" == expSplit || "hash_percent" == expSplit )
      {
         if( true == commIsStandalone( db ) )
            throw "standalone" ;
         var rg = commGetGroups( db ) ;
         if( 1 >= rg.length )
            throw "one datagroup" ;
         var expCS = db.getCS( COMMCSNAME ) ;
         var expCL = expCS.getCL( COMMCLNAME ) ;
         println( 'alter( {"ShardingKey":{"OID":-1},'+
                  '"ShardingType":"hash","Partition":2048} ) ') ;
         expCL.alter( {"ShardingKey":{"OID":-1}, "ShardingType":"hash",
                       "Partition":2048} ) ;
         println( "alter over" ) ;
         if( "hash" == expSplit )
         {
            toolCollectionSplit( db, COMMCSNAME, COMMCLNAME, expSplit,
                                 { "Partition": 512 }, { "Partition": 1024 } ) ;
         }
         else
         {
            // hash and percent split
            toolCollectionSplit( db, COMMCSNAME, COMMCLNAME, "percent", 30 ) ;
         }
         println( "success to do " + expSplit + " split in export collection" ) ;
      }

      // export data
      if( "" == LocalPath || "" == InstallPath )
      {
         println( "local path : " + LocalPath + "; install path : " +
                  InstallPath ) ;
         throw "path cannot be null" ;
      }
      cmd.run( "rm -rf " + exportFile ) ;
      println( "remover file success" ) ;
      cmd.run( exportCmd ) ;
      println( ">export successful" ) ;
      // import data to new collection
      var cl = commCreateCL( db, COMMCSNAME, newCLNAME, 0, true, true, false,
                             "create CL in the beginning" ) ;
      /*** hash split in import ***/
      if( "hash" == impSplit || "hash_percent" == impSplit )
      {
         if( true == commIsStandalone( db ) )
            throw "standalone" ;
         var rg = commGetGroups( db ) ;
         if( 1 >= rg.length )
            throw "one datagroup" ;
         var impCS = db.getCS( COMMCSNAME ) ;
         var impCL = impCS.getCL( newCLNAME ) ;
         impCL.alter( {"ShardingKey":{"OID":1}, "ShardingType":"hash",
                       "Partition":2048} ) ;
         if( "hash" == impSplit )
         {
            toolCollectionSplit( db, COMMCSNAME, newCLNAME, impSplit,
                                 { "Partition": 512 }, { "Partition": 1024 } ) ;
         }
         else
         {
            println( "when collection don't have data, percent split will failed" ) ;
            // hash and percent split
            toolCollectionSplit( db, COMMCSNAME, newCLNAME, "percent", 70 ) ;
         }
         println( "success to do " + impSplit + " split in import collection" ) ;
      }
      cmd.run( importCmd ) ;
      cmd.run( "rm -rf " + exportFile ) ;
      println( ">import successful" ) ;

      // verify the result of export
      var fileName = CSPREFIX + "_exportImport_newCL.file" ;
      toolVerifyLobs( cl, lobNum, fileName ) ;
      println( ">verify successful" ) ;
      // drop collection in the end
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "clean collection in the end, correct" ) ;
      commDropCL( db, COMMCSNAME, newCLNAME, false, false,
                  "clean collection in the end, correct" ) ;
   }
   catch( e )
   {
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                 "clean collection in the end, wrong" ) ;
      commDropCL( db, COMMCSNAME, newCLNAME, true, true,
                  "clean collection in the end, correct" ) ;
      if( "standalone" == e )
         println( "run mode is standalone" ) ;
      else if( "one datagroup" == e )
         println( "the cluster only have one datagroup" ) ;
      else
      {
         println( "failed to test export and import parameters, rc = " + e ) ;
         throw e ;
      }
   }
}

/******************************************************************************
*@Description : main test API for migration testcase.
*@Parameter :
*             lobNum: the number of lobs that insert
*             migCmd: the command of migrate lob
*             migFile: the migration file
*             migCLNAME: the collection name that migrate lob to the collection
*             srcMigSplit: migrate out lob collection split type
*             dstMigSplit: migrate in lob collection split type
*             migdb: new db conection for migration destination collection
******************************************************************************/
function testMigrationPara( db, lobNum, migCmd, migFile,
                            migCLNAME, srcMigSplit, dstMigSplit, migdb )
{
   if( undefined == srcMigSplit ){ srcMigSplit = null ; }
   if( undefined == dstMigSplit ){ dstMigSplit = null ; }
   if( undefined == migdb ){ migdb = db }
   try {
      // put lob data
      var fileName = CSPREFIX + "_migrationFile.txt" ;
      var lobNum = 100 ;
      toolPutLobs( db, fileName, lobNum ) ;
      println( ">succes to insert " + lobNum + " records" ) ;
      /*** hash split ***/
      if( "hash" == srcMigSplit || "hash_percent" == dstMigSplit )
      {
         if( true == commIsStandalone( db ) )
            throw "standalone" ;
         println( "error, here split" ) ;
         var rg = commGetGroups( db ) ;
         if( 1 >= rg.length )
            throw "one datagroup" ;
         var expCS = db.getCS( COMMCSNAME ) ;
         var expCL = expCS.getCL( COMMCLNAME ) ;
         expCL.alter( {"ShardingKey":{"OID":-1}, "ShardingType":"hash",
                       "Partition":2048} ) ;
         if( "hash" == srcMigSplit )
         {
            toolCollectionSplit( db, COMMCSNAME, COMMCLNAME, srcMigSplit,
                                 { "Partition": 512 }, { "Partition": 1024 } ) ;
         }
         else
         {
            // hash and percent split
            toolCollectionSplit( db, COMMCSNAME, COMMCLNAME, "percent", 30 ) ;
         }
         println( "success to do " + srcMigSplit + " split in export collection" ) ;
      }

      // migration data
      if( "" == LocalPath || "" == InstallPath )
      {
         println( "local path : " + LocalPath + "; install path : " +
                  InstallPath ) ;
         throw "path cannot be null" ;
      }
      // destnation migration collection
      var cl = commCreateCL( migdb, COMMCSNAME, migCLNAME, 0, true, true, false,
                             "create CL in the beginning" ) ;
      /*** hash split in import ***/
      if( "hash" == dstMigSplit || "hash_percent" == dstMigSplit )
      {
         if( true == commIsStandalone( db ) )
            throw "standalone" ;
         var rg = commGetGroups( db ) ;
         if( 1 >= rg.length )
            throw "one datagroup" ;
         var dstMigCS = migdb.getCS( COMMCSNAME ) ;
         var dstMigCL = dstMigCS.getCL( migCLNAME ) ;
         dstMigCL.alter( {"ShardingKey":{"OID":1}, "ShardingType":"hash",
                       "Partition":2048} ) ;
         if( "hash" == dstMigSplit )
         {
            toolCollectionSplit( migdb, COMMCSNAME, migCLNAME, dstMigSplit,
                                 { "Partition": 512 }, { "Partition": 1024 } ) ;
         }
         else
         {
            println( "when collection don't have data, percent split will failed" ) ;
            // hash and percent split
            toolCollectionSplit( migdb, COMMCSNAME, migCLNAME, "percent", 70 ) ;
         }
         println( "success to do " + dstMigSplit + " split in import collection" ) ;
      }
      cmd.run( migCmd ) ;
      cmd.run( "rm -rf " + migFile ) ;
      println( ">migration successful" ) ;

      // verify the result of export
      var fileName = CSPREFIX + "_migration_newCL.txt" ;
      toolVerifyLobs( cl, lobNum, fileName ) ;
      println( ">verify successful" ) ;
      // drop collection in the end
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "clean collection in the end, correct" ) ;
      commDropCL( migdb, COMMCSNAME, migCLNAME, false, false,
                  "clean collection in the end, correct" ) ;
   }
   catch( e )
   {
      println( "failed to test migration parameters, rc = " + e ) ;
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                 "clean collection in the end, wrong" ) ;
      commDropCL( migdb, COMMCSNAME, migCLNAME, true, true,
                  "clean collection in the end, correct" ) ;
      if( "standalone" == e )
         println( "run mode is standalone" ) ;
      else if( "one datagroup" == e )
         println( "the cluster only have one datagroup" ) ;
      else
         throw e ;
   }
}

function testToolAnomalyPara( db, lobNum, toolCmd, toolFile, newCLNAME,
                              srcMigSplit, dstMigSplit, newDB, mode )
{
   if( undefined == srcMigSplit ){ srcMigSplit = null ; }
   if( undefined == dstMigSplit ){ dstMigSplit = null ; }
   if( undefined == newDB || null == newDB ){ newDB = db ; }
   try
   {
      if( "export" == mode || "migration" == mode )
      {
         commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                     "clean new collection in the beginning" ) ;
         // put lob data
         var fileName = CSPREFIX + "_toolFile.txt" ;
         var lobNum = 100 ;
         toolPutLobs( db, fileName, lobNum ) ;
         println( ">succes to insert " + lobNum + " records" ) ;
         /*** hash split ***/
         if( "range" == srcMigSplit || "range_percent" == srcMigSplit )
         {
            if( true == commIsStandalone( db ) )
               throw "standalone" ;
            var rg = commGetGroups( db ) ;
            if( 1 >= rg.length )
               throw "one datagroup" ;
            var expCS = db.getCS( COMMCSNAME ) ;
            var expCL = expCS.getCL( COMMCLNAME ) ;
            expCL.alter( {"ShardingKey":{"OID":-1}, "ShardingType":"range" } ) ;
            if( "range" == srcMigSplit )
            {
               toolCollectionSplit( db, COMMCSNAME, COMMCLNAME, srcMigSplit,
                                    { "key": 512 }, { "key": 1024 } ) ;
            }
            else
            {
               // hash and percent split
               toolCollectionSplit( db, COMMCSNAME, COMMCLNAME, "percent", 40 ) ;
            }
            println( "success to do " + srcMigSplit + " split in export collection" ) ;
         }
      }
      // inspect the local path and install path
      if( "" == LocalPath || "" == InstallPath )
      {
         println( "local path : " + LocalPath + "; install path : " +
                  InstallPath ) ;
         throw "path cannot be null" ;
      }
      // destnation migration collection
      if( "import" == mode || "migration" == mode )
      {
         if( COMMCLNAME != newCLNAME )
         {
            commDropCL( db, COMMCSNAME, newCLNAME, true, true,
                        "clean new collection in the beginning" ) ;
         }
         if( -1 != newCLNAME )
         {
            var cl = commCreateCL( newDB, COMMCSNAME, newCLNAME, 0, true, true,
                                   false, "create CL in the beginning" ) ;
         }
         else
         {
            var cs = db.getCS( COMMCSNAME ) ;
            var cl = cs.getCL( COMMCLNAME ) ;
         }
         /*** hash split in import ***/
         if( "range" == dstMigSplit || "range_percent" == dstMigSplit )
         {
            if( true == commIsStandalone( db ) )
               throw "standalone" ;
            var rg = commGetGroups( db ) ;
            if( 1 >= rg.length )
               throw "one datagroup" ;
            var dstMigCS = newDB.getCS( COMMCSNAME ) ;
            var dstMigCL = dstMigCS.getCL( newCLNAME ) ;
            dstMigCL.alter( {"ShardingKey":{"OID":1}, "ShardingType":"range" } ) ;
            if( "hash" == dstMigSplit )
            {
               toolCollectionSplit( newDB, COMMCSNAME, newCLNAME, dstMigSplit,
                                    { "key": 500 }, { "key": 1000 } ) ;
            }
            else
            {
               println( "when collection don't have data, percent split will failed" ) ;
               // hash and percent split
               //toolCollectionSplit( newDB, COMMCSNAME, newCLNAME, "percent", 70 ) ;
            }
            println( "success to do " + dstMigSplit + " split in import collection" ) ;
         }
      }

      if( "export" == mode )
      {
         // export data
         cmd.run( "rm -rf " + toolFile ) ;
         println( "remover file success" ) ;
         cmd.run( toolCmd ) ;
         println( ">export successful" ) ;
      }
      else
      {
         // import data and migration
         cmd.run( toolCmd ) ;
         cmd.run( "rm -rf " + toolFile ) ;
         println( ">" + mode + " data successful" ) ;
      }

      // verify the result of export
      if( "import" == mode || "migration" == mode )
      {
         var fileName = CSPREFIX + "_anomaly_newCL.txt" ;
         toolVerifyLobs( cl, lobNum, fileName ) ;
         println( ">verify successful" ) ;
         commDropCL( newDB, COMMCSNAME, newCLNAME, false, false,
                     "clean collection in the end, correct" ) ;
      }
      // drop collection in the end
      if( "export" == mode || "migration" == mode )
      {
         commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                     "clean collection in the end, correct" ) ;
      }
   }
   catch( e )
   {
      println( "failed to test anormaly sdblobtool parameters, rc = " + e ) ;
      if( "export" == mode || "migration" == mode )
      {
         commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                    "clean collection in the end, wrong" ) ;
      }
      if( "import" == mode || "migration" == mode )
      {
         commDropCL( db, COMMCSNAME, newCLNAME, true, true,
                    "clean collection in the end, wrong" ) ;
      }
      if( "standalone" == e )
         println( "run mode is standalone" ) ;
      else if( "one datagroup" == e )
         println( "the cluster only have one datagroup" ) ;
      else
         throw e ;
   }
}
/******************************************************************************
*@Description : the number of write lob
******************************************************************************/
function toolPutLobs( db, fileName, lobNum )
{
   try
   {
      var lobfile = LocalPath + "/" + fileName ;
      // global
      file = new File( lobfile ) ;
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                             "create CL in the beginning" ) ;
      var loopNum = 100 ;
      //println( "install path = " + InstallPath ) ;
      var content = null ;
      for( var i = 0 ; i < loopNum ; ++i )
      {
         content = content + i + "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
      }
      file.write( content ) ;
      file.close() ;
      // global variable write lob md5.The best way is write MD5
      // by myself program
      var wMd5sum = cmd.run( "md5sum " + lobfile ).split( " " ) ;
      wMd5 = wMd5sum[0] ;
      Cursor = new Array() ;
      for( var i = 0 ; i < lobNum ; ++i )
      {
         // global variable
         Cursor[i] = cl.putLob( lobfile ) ;
         cl.insert( { "OID" : Cursor[i] } ) ;
      }
      var cnt = 0 ;
      do
      {
         ++cnt ;
      }while( lobNum != cl.count() && 1000 >= cnt ) ;
      if( lobNum != cl.count() )
      {
         println( "lob have count : " + cl.count() +
                  " and not equal lob numbers" + lobNum ) ;
      }
      cmd.run( "rm -rf " + lobfile ) ;
   }
   catch( e )
   {
      println( "failed put lobs, rc = " + e  ) ;
      throw e ;
   }
}

/******************************************************************************
*@Description : verify import/migrate lob is correct or not
******************************************************************************/
function toolVerifyLobs(  cl, lobNum, file )
{
   try
   {
      var lobCount = cl.count() ;   // only export lob, not record
      var listCount = cl.listLobs().toArray() ;
      var cnt = 0 ;
      while( parseInt(lobNum) != parseInt(listCount.length) && 1000 > cnt )
      {
         listCount = cl.listLobs().toArray() ;
         cnt++ ;
         sleep( 3 ) ;
      }
      if( parseInt(lobNum) != parseInt(listCount.length) && 0 == lobCount )
      {
         println( "the number of lob put = " + lobNum ) ;
         println( "the number of lob read = " + listCount.length ) ;
         println( "verify collection : " + cl ) ;
         throw "the number of lobs is not equal" ;
      }
      for( var i = 0 ; i < listCount.length ; ++i )
      {
         var getLobFile = LocalPath + "/" + file ;
         cl.getLob( Cursor[i], getLobFile, true ) ;
         var rMd5sum = cmd.run( "md5sum " + getLobFile ).split( " " ) ;
         rMd5 = rMd5sum[0] ;
         cmd.run( "rm -rf " + getLobFile ) ;
         if( wMd5 != rMd5 )
         {
            println( "write lob md5 : " + wMd5 ) ;
            println( "read lob md5 : " + rMd5 ) ;
            throw "md5 was not equal" ;
         }
      }
   }
   catch( e )
   {
      println( "failed to verify lobs, rc = " + e ) ;
      throw e ;
   }
}

/******************************************************************************
*@Description : split collection by using hash/percent/range
*@Parameters :
*               splitType: the type of split colletion[hash/percent/range]
******************************************************************************/
function toolCollectionSplit( db, csName, clName, splitType, condtion1, condtion2 )
{
   if( "hash" != splitType && "range" != splitType && "percent" != splitType )
   {
      splitType = "hash" ;
   }
   try
   {
      var fullCLNAME = csName + "." + clName ;
      var clRG = commGetCLGroups( db, fullCLNAME ) ;
      var rg = commGetGroups( db ) ;
      var srcRg = clRG[0] ;
      var dstRg = null ;
      for( var i = 0 ; i < rg.length ; ++i )
      {
         if( srcRg != rg[i][0].GroupName)
         {
            dstRg = rg[i][0].GroupName ;
            break ;
         }
      }
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      println( "source group name : " + srcRg ) ;
      println( "destnation group name : " + dstRg ) ;
      if( undefined == srcRg || undefined == dstRg)
         throw "get error groups" ;
      if( "hash" == splitType || "range" == splitType )
      {
         // hash split and range split
         println( splitType + " begin" ) ;
         cl.split( srcRg, dstRg, condtion1, condtion2 ) ;
      }
      else
      {
         // percent split
         println( splitType + " begin" ) ;
         cl.split( srcRg, dstRg, condtion1 ) ;
      }
   }
   catch( e )
   {
      println( "failed to split collectio by: " + splitType + ", rc = " + e ) ;
      throw e ;
   }
}

