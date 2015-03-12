/******************************************************************************
@Description : Test the hint index, the hint index is not exist.
@Modify list :
               2014-6-12  xiaojun Hu  Init
******************************************************************************/
function main()
{
   indexName = CSPREFIX + "hintidx" ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                          "create CL in the beginning." ) ;
   // Insert data to Sdb
   insertData( db, COMMCSNAME, COMMCLNAME ) ;
   //if the index is not exsist, it will auto find other index
   createIndex( cl, indexName, { "no" : -1 }, false, false ) ;
   hintInspectIndex( cl, indexName, "no", -1, false, false ) ;
   var clName = COMMCSNAME + "." + COMMCLNAME ;
   try
   {
      var data_Idx1 = hintSnapshotSession( db, clName ) ;
      // Test Point : query part
      println( cl.find({no:{$lte:10000},score:{$gte:9999}}).hint({"":indexName}) ) ;
      var data_Idx2 = hintSnapshotSession( db, clName ) ;
      // The index will not be changed
      if ( data_Idx2[0] > data_Idx1[0] && data_Idx2[1] > data_Idx1[1] )
      {
         println( "previous data read = " + data_Idx1[0] ) ;
         println( "final data read = " + data_Idx2[0] ) ;
         println( "previous index read = " + data_Idx1[1] ) ;
         println( "final index read = " + data_Idx2[1] ) ;
         println("Success to hint via index") ;
      }
      else
      {
         println( "previous data read = " + data_Idx1[0] ) ;
         println( "final data read = " + data_Idx2[0] ) ;
         println( "previous index read = " + data_Idx1[1] ) ;
         println( "final index read = " + data_Idx2[1] ) ;
         throw "Failed to query by hint, one" ;
      }
      // Test Point : query part
      println( cl.find({no:{$lte:9001},score:{$gte:9000}}).hint({"":"noexistname"}) ) ;
      var data_Idx3 = hintSnapshotSession( db, clName ) ;
      if ( data_Idx3[0] > data_Idx2[0] && data_Idx3[1] > data_Idx2[1] )
      {
         println( "previous data read = " + data_Idx2[0] ) ;
         println( "final data read = " + data_Idx3[0] ) ;
         println( "previous index read = " + data_Idx2[1] ) ;
         println( "final index read = " + data_Idx3[1] ) ;
      }
      else
      {
         println( "previous data read = " + data_Idx2[0] ) ;
         println( "final data read = " + data_Idx3[0] ) ;
         println( "previous index read = " + data_Idx2[1] ) ;
         println( "final index read = " + data_Idx3[1] ) ;
         throw "failed to query by hint, two" ;
      }
   }
   catch( e )
   {
      throw e ;
   }
}

// Running
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop collection in the beginning" ) ;
   main( db ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop collection in the end, correct" ) ;
   db.close() ;
}
catch( e )
{
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
    //           "drop collection in the end, wrong" ) ;
   db.close() ;
   throw e ;
}
