/******************************************************************************
*@Description : test export and import's parameters: --ignorefe
*               --ignorefe true === --ignorefe false
*@Modify list :
*               2014-11-19   xiaojun Hu  Init
******************************************************************************/
function testExportImportParaIgnore( db, lobNum, exportCmd, exportFile,
                               importCmd, newCLNAME, expSplit, impSplit )
{
   if( undefined == expSplit ){ expSplit = null ; }
   if( undefined == impSplit ){ impSplit = null ; }
   try {
      // put lob data
      var fileName = CSPREFIX + "_exportFile.txt" ;
      var lobNum = 100 ;
      toolPutLobs( db, fileName, lobNum ) ;
      println( ">succes to insert " + lobNum + " records" ) ;
      /*** hash split ***/
      if( "hash" == expSplit || "hash_percent" == expSplit )
      {
         var expCS = db.getCS( COMMCSNAME ) ;
         var expCL = expCS.getCL( COMMCLNAME ) ;
         expCL.alter( {"ShardingKey":{"OID":-1}, "ShardingType":"hash",
                       "Partition":2048} ) ;
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
      var lobCur = cl.listLobs().toArray() ;
      var old_tm = new Array() ;
      var j = 0 ;
      for( var i = 0 ; i < lobCur.length ; ++i )
      {
         var lobOid = eval( "(" + lobCur[i] + ")" ) ;
         if( i < lobCur.length/2 )
         {
            cl.deleteLob( lobOid.Oid.$oid ) ;   // delete Lob
         }
         else
         {
            ++j ;
            old_tm[j] = lobOid.CreateTime.$timestamp ;
         }
      }
      sleep(5000) ;
      cmd.run( importCmd ) ;
      cmd.run( "rm -rf " + exportFile ) ;
      println( ">import successful" ) ;

      // verify the result of export
      var fileName = CSPREFIX + "_ignorefe_newCL.file" ;
      toolVerifyLobs( cl, lobNum, fileName ) ;
      var lobCur = cl.listLobs().toArray() ;
      var new_tm = new Array() ;
      var tmcnt = 0 ;
      for( var i = 0 ; i < lobCur.length ; ++i )
      {
         var lobOid = eval( "(" + lobCur[i] + ")" ) ;
         new_tm[i] = lobOid.CreateTime.$timestamp ;
         for( var j = 0 ; j < old_tm.length ; ++j )
         {
            if( new_tm[i] == old_tm[j] )
            {
               //println( "count : " + tmcnt ) ;
               tmcnt++ ;
               break ;
            }
         }
      }
      if( lobCur.length/2 != tmcnt )
      {
         println( "expect : " + lobCur.length/2 + "; actural" + tmcnt ) ;
         throw "faile to run ingnore parameter" ;
      }
      println( "equal number : " + tmcnt ) ;
      println( ">verify successful" ) ;
      // drop collection in the end
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "clean collection in the end, correct" ) ;
      commDropCL( db, COMMCSNAME, newCLNAME, false, false,
                  "clean collection in the end, correct" ) ;
   }
   catch( e )
   {
      println( "failed to test export and import parameters, rc = " + e ) ;
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                 "clean collection in the end, wrong" ) ;
      throw e ;
   }
}

function main( db )
{
   // init global variable
   initGlobalVar() ;
   // put lob data
   var lobNum = 100 ;
   var filename = CSPREFIX + "_export_ignorefe.file" ;
   var exportFile = LocalPath + "/" + filename ;
   var expFullCL = COMMCSNAME + "." + COMMCLNAME ;
   var newCLNAME = COMMCLNAME + "_new" ;
   var impFullCL = COMMCSNAME + "." + newCLNAME ;
/******************************************************************************
*@Test Description:  export and import parameter, remote hostname and svcname.
*                    group need have many hosts.
*  import: ./sdblobtool --hostname XXX --svcname XXX --operation import
*                       --collection XXX --file  XXX/XXX --ignorefe true
*******************************************************************************/
   //var expSplit = "hash_percent" ;
   //var impSplit = "hash_percent" ;
   var expSplit = null ;
   var impSplit = null ;
   var exportCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation export" +
                   " --collection " + expFullCL + " --file " + exportFile ;
   var importCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation import" +
                   " --collection " + impFullCL + " --file " + exportFile +
                   " --ignorefe true" ;
   println( "==>>[TP1]test export command : " + exportCmd ) ;
   println( "==>>[TP1]test import command : " + importCmd ) ;
   testExportImportParaIgnore( db, lobNum, exportCmd, exportFile,
                               importCmd, newCLNAME, expSplit, impSplit ) ;
/******************************************************************************
*@Test Description:  export and import parameter, remote hostname and svcname.
*                    group need have many hosts.
*  import: ./sdblobtool --hostname XXX --svcname XXX --operation import
*                       --collection XXX --file  XXX/XXX --ignorefe false
*******************************************************************************/
   //var expSplit = "hash_percent" ;
   //var impSplit = "hash_percent" ;
   var expSplit = null ;
   var impSplit = null ;
   var exportCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation export" +
                   " --collection " + expFullCL + " --file " + exportFile ;
   var importCmd = InstallPath + "/bin/sdblobtool --hostname " + COORDHOSTNAME +
                   " --svcname " + COORDSVCNAME + " --operation import" +
                   " --collection " + impFullCL + " --file " + exportFile +
                   " --ignorefe false" ;
   println( "==>>[TP2]test export command : " + exportCmd ) ;
   println( "==>>[TP2]test import command : " + importCmd ) ;
   testExportImportParaIgnore( db, lobNum, exportCmd, exportFile,
                               importCmd, newCLNAME, expSplit, impSplit ) ;

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
