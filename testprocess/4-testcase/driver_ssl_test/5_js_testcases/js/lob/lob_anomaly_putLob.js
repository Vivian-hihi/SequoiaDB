/******************************************************************************
*@Description : anomaly test that for putLob/listLob/deleteLob
*@Modify list :
*               2014-12-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var testFile = CSPREFIX + "lobTest.file",
       cmd = new Cmd() ;

   lobAutoFile( testFile ) ;   // auto file
   // create collection
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, true,
                          "create collection" ) ;
   // put lob with no lob file[Test_Point_1]
   try
   {
      var list = cl.listLobs( testFile ) ;
      cl.putLob() ;
      throw "ErrExecuteLob" ;
   }
   catch( e )
   {
      if( "Error: SdbCollection.putLob(): wrong arguments" != e &&
          "Error: SdbCollection.listLobs(): wrong arguments" != e )
      {
         println( "failed to execute put lob with no file, rc = " + e ) ;
         throw e ;
      }
      else
         println( "success to execute putLob with no lob file" ) ;
   }
   // delete lob with no oid[Test_Point_2]
   try
   {
      // delete lobs
      cl.deleteLob() ;
      throw "ErrorDeleteLob" ;
   }
   catch( e )
   {
      if( "Error: SdbCollection.deleteLob(): wrong arguments" != e )
      {
         // remove lobfile
         cmd.run( "rm -rf " + testFile ) ;
         println( "failed to execute delete lob with no file, rc = " + e ) ;
         throw e ;
      }
      else
      {
         // remove lobfile
         cmd.run( "rm -rf " + testFile ) ;
         println( "success to execute deleteLob with no parameter, rc = " ) ;
      }
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
