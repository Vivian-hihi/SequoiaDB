/******************************************************************************
*@Description : the collection do autosplit in domain.test input normal record
*               and lob data into collection
*@Modify list :
*               2014-12-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var putNum = 100,
       oid = new Array(),
       partitionNum = 2048;

   lobAutoFile( testFile ) ;   // auto file
   // create domain
   try
   {
      var rg = lobGetGroups( db ) ;
      db.createDomain( domName, rg, { "AutoSplit": true } ) ; // create domain
      println( "success to create domain" ) ;
      lobCreateCS( db, DOMCSNAME, domName ) ;
      println( "success to create collection space attach domain" ) ;
   }
   catch( e )
   {
      println( "failed to create domain and CS, rc = " + e ) ;
      throw e ;
   }
   // create collection
   var optionObj = { "ShardingKey":{"no":1}, "ShardingType":"hash", "ReplSize":0,
                     "Partition":partitionNum, "Compressed":true } ;
   var cl = commCreateCLByOption( db, DOMCSNAME, COMMCLNAME, optionObj, true,
                                  true, "create collection for hash split" ) ;
   // put normal data and lob data
   try
   {
      lobInsert( cl, putNum ) ;
      println( "success to put normal record data" ) ;
      oid = lobPutLob( cl, testFile, putNum ) ;
      println( "success to put lob data" ) ;
   }
   catch( e )
   {
      println( "failed to normal record and lob data in collection, rc = " +e ) ;
      throw e ;
   }
   // get lob
   try
   {
      for( var i = 0 ; i < oid.length ; ++i )
      {
         cl.getLob( oid[i], getTestFile, true ) ;
      }
      println( "success to get lob" ) ;
      for( var i = 0 ; i < cl.count() ; ++i )
      {
         var count = cl.find( {"no":i} ).count() ;
         if( 1 != count )
         {
            println( "failed to query data, rc = " + cl.find( {"no":i} ) ) ;
            throw "ErrNumberQuery" ;
         }
      }
      println( "success to query" ) ;
   }
   catch( e )
   {
      println( "failed to get lob and query nomral data, rc = " + e ) ;
      throw e ;
   }
}

// Run Main
try
{
   var testFile = CSPREFIX + "lobTest.file",
       getTestFile = CSPREFIX + "lobTestGet.file",
       DOMCSNAME = CSPREFIX + "_domainCS",
       domName = CSPREFIX + "_domName",
       cmd = new Cmd() ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clear collection in the beginning" ) ;
   if( false != commIsStandalone( db ) )
      throw "standalone" ;
   main( db ) ;
   // clean environment
   commDropCS( db, DOMCSNAME, false, "drop collection in the end, correct" ) ;
   println( "drop collection space in domain success" ) ;
   db.dropDomain( domName ) ;
   println( "drop domain success" ) ;
   // remove lobfile
   cmd.run( "rm -rf " + testFile ) ;
   cmd.run( "rm -rf " + getTestFile ) ;
   db.close( ) ;
}
catch( e )
{
   if( "standalone" != e )
   {
      commDropCS( db, DOMCSNAME, true, "drop collection in the end, correct" ) ;
      db.dropDomain( domName ) ;
      // remove lobfile
      cmd.run( "rm -rf " + testFile ) ;
      cmd.run( "rm -rf " + getTestFile ) ;
      db.close( ) ;
      //throw "heere: " + e ;
   }
   else
      println( "runmode is standalone" ) ;
}
