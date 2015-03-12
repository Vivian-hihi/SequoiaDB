/******************************************************************************
*@Description : the collection do hash split befor input data.test input
*               normal record and lob data into collection
*@Modify list :
*               2014-12-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var testFile = CSPREFIX + "lobTest.file",
       getTestFile = CSPREFIX + "lobTestGet.file",
       putNum = 100,
       oid = new Array(),
       partitionNum = 2048,
       cmd = new Cmd() ;

   lobAutoFile( testFile ) ;   // auto file
   // create collection
   var optionObj = { "ShardingKey":{"no":1}, "ShardingType":"hash", "ReplSize":0,
                     "Partition":partitionNum, "Compressed":true } ;
   var cl = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, optionObj, true,
                                  true, "create collection for hash split" ) ;
   // collection do hash split before put data
   try
   {
      if( false != commIsStandalone(db) )
         throw "run mode is standalone" ;
      var FULLCLNAME = COMMCSNAME + "." + COMMCLNAME ;
      var clRg = commGetCLGroups( db, FULLCLNAME ),
          rg = lobGetGroups( db ) ;
      println( "collection located in group: " + rg ) ;
      if( 1 != clRg.length )
      {
         println( "collection have more than one groups: " + clRg ) ;
         throw "NotOneGroup" ;
      }
      if( 1 != rg.length )
         throw "database only have one group" ;
      var cond = partitionNum/rg.length ;
      //println( "the group length: " + cond ) ;
      var loopCond = cond ;

      for( var i = 0 ; i < rg.length ; ++i )
      {
         if( clRg[0] != rg[i] )
         {
            var firstCond = { "Partition": loopCond-cond } ;
            var secondCond = { "Partition": loopCond } ;
            lobSplit( cl, clRg[0], rg[i], firstCond, secondCond ) ;
            loopCond += cond ;
         }
      }

      println( "success to split collection" ) ;
   }
   catch( e )
   {
      if( "run mode is standalone" != e &&
          "database only have one group" != e )
      {
         println( "failed to split collection, rc = " + e ) ;
         throw e ;
      }
   }
   // put normal data and lob data
   try
   {
      lobInsert( cl, putNum ) ;
      oid = lobPutLob( cl, testFile, putNum ) ;
      println( "success to put normal data and lob data" ) ;
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
      // remove lobfile
      cmd.run( "rm -rf " + testFile ) ;
      cmd.run( "rm -rf " + getTestFile ) ;
   }
   catch( e )
   {
      // remove lobfile
      cmd.run( "rm -rf " + testFile ) ;
      cmd.run( "rm -rf " + getTestFile ) ;
      println( "failed to get lob and query nomral data, rc = " + e ) ;
      throw e ;
   }
}

// Run Main
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clear collection in the beginning" ) ;
   main( db ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop collection in the end, correct" ) ;
   db.close( ) ;
}
catch( e )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop collection in the end , error" ) ;
   db.close( ) ;
   throw e ;
}
