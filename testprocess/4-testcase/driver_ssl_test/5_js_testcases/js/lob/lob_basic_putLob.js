/******************************************************************************
*@Description : test function: db.collectionspace.collection.putLob(<file>)
*               db.collectionspace.collection.listLobs()
*               db.collectionspace.collection.deleteLob(<oid>)
*@Modify list :
*               2014-12-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var testFile = CSPREFIX + "lobTest.file",
       putNum = 100,
       oid = new Array(),
       cmd = new Cmd() ;
   lobAutoFile( testFile ) ;   // auto file
   // create collection
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, true,
                          "create collection in the beginning" ) ;
   // put lob file in collection[Test Point]
   try
   {
      for( var i = 0 ; i < putNum ; ++i )
      {
         oid[i] = cl.putLob( testFile ) ;
      }
      println( "success to put lob" ) ;
      // simple verify
      var listLob = cl.listLobs().toArray() ;
      if( putNum != listLob.length )
      {
         println( "expect lob number: " + putNum ) ;
         println( "actual lob number: " + listLob.length ) ;
         throw "WrongLobNumber" ;
      }
      println( "success to put lob in colleciton" ) ;
      // delete lobs
      for( var i = 0 ; i < oid.length ; ++i )
      {
         cl.deleteLob( oid[i] ) ;
      }
      println( "success to delete lob in colleciton" ) ;
      // remove lobfile
      cmd.run( "rm -rf " + testFile ) ;
   }
   catch( e )
   {
      cmd.run( "rm -rf " + testFile ) ;
      println( "failed to put lob in collection, rc = " +e ) ;
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
