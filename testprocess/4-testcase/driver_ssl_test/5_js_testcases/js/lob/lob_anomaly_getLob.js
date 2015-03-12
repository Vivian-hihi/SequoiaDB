/******************************************************************************
*@Description : anomaly test for getLob
*@Modify list :
*               2014-12-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var testFile = CSPREFIX + "lobTest.file",
       getTestFile = CSPREFIX + "lobTestGet.file",
       putNum = 10,
       cmd = new Cmd() ;

   lobAutoFile( testFile ) ;
   // create collection
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, true,
                          "create collection" ) ;
   // put Lob
   var oid = lobPutLob( cl, testFile ) ;   //Array
   // get lob with no parmameter : getLob()[Test_Point_1]
   try
   {
      cl.getLob() ;
   }
   catch( e )
   {
      if( "Error: SdbCollection.getLob(): wrong arguments" != e )
      {
         println( "failed to execute get lob with no file, rc = " + e ) ;
         throw e ;
      }
      else
      {
         println( "success to execute getLob with no parameter, rc = " ) ;
      }
   }
   // get lob specify same file and don't set forced : true[Test_Point_2]
   try
   {
      for( var i = 0 ; i < oid.length ; ++i )
      {
         cl.getLob( oid[i], getTestFile ) ;
         if( 0 < i )
            throw "ErrorExecuteGetLob" ;
      }
   }
   catch( e )
   {
      if( -5 != e )
      {
         println( "failed to execute get lob with no parameter:" +
                  " forced, rc = " + e ) ;
         throw e ;
      }
      else
      {
         println( "success to execute getLob with no parameter:'forced', rc = " ) ;
      }
   }
   // delete lob
   try
   {
      // delete lobs
      for( var i = 0 ; i < oid.length ; ++i )
      {
         cl.deleteLob( oid[i] ) ;
      }
      println( "success to delete lob in colleciton" ) ;
      // remove lobfile
      cmd.run( "rm -rf " + testFile ) ;
      cmd.run( "rm -rf " + getTestFile ) ;
   }
   catch( e )
   {
      // remove lobfile
      cmd.run( "rm -rf " + testFile ) ;
      cmd.run( "rm -rf " + getTestFile ) ;
      println( "failed to delete lob, rc = " + e ) ;
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
