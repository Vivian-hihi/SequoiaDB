/******************************************************************************
*@Description : test same collection input nomal record and lob data
*@Modify list :
*               2014-12-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var testFile = CSPREFIX + "lobTest.file",
       getTestFile = CSPREFIX + "lobTestGet.file",
       putNum = 100,
       oid = new Array(),
       cmd = new Cmd() ;

   lobAutoFile( testFile ) ;   // auto file
   // create collection
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, true,
                          "create collection" ) ;
   // put normal data and put lob
   try
   {
      oid = lobPutLob( cl, testFile, putNum ) ;
      println( "success to put lob" ) ;
      lobInsert( cl, putNum ) ;
      println( "success to insert normal record" ) ;
   }
   catch( e )
   {
      println( "failed to put lob and normal record in collection, rc = " +e ) ;
      throw e ;
   }
   // get lob
   try
   {
      for( var i = 0 ; i < oid.length ; ++i )
      {
         cl.getLob( oid[i], getTestFile, true ) ;
      }
      println( "success to get lob data" ) ;
   }
   catch( e )
   {
      println( "failed to get lob, rc = " + e ) ;
      throw e ;
   }
   // query data
   try
   {
      for( var i = 0 ; i < cl.count() ; ++i )
      {
         var count = cl.find( {"no":i} ).count() ;
         if( 1 != count )
         {
            println( "failed to query data, rc = " + cl.find( {"no":i} ) ) ;
            throw "ErrNumberQuery" ;
         }
      }
      println( "success to query data" ) ;
      cmd.run( "rm -rf " + testFile ) ;
   }
   catch( e )
   {
      cmd.run( "rm -rf " + testFile ) ;
      println( "failed to query data, rc = " + e ) ;
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
