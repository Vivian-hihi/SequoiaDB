/******************************************************************************
*@Description : Test the hint index by using index read.
*@Modify list :
*               2014-6-12   xiaojun Hu  Init
*               2014-11-10  xiaojun Hu  Change
******************************************************************************/
function main( db )
{
   indexName = CSPREFIX + "Idx" ;
   // Drop CL in the beginning
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                          "create collection in the beginning." ) ;
   // Insert data to Sdb
   insertData( db, COMMCSNAME, COMMCLNAME ) ;
   var clName = COMMCSNAME + "." + COMMCLNAME ;
   try
   {
      var data_Idx1 = hintSnapshotSession( db, clName ) ;
      // Hint not exist index and no indexes in collection[T_Point1]
      println( cl.find({no:{$lt:5002},score:{$gte:5000}}).hint({"":""}) ) ;
      var data_Idx2 = hintSnapshotSession( db, clName ) ;
      // The index will not be changed
      if ( data_Idx2[0] > data_Idx1[0] && data_Idx2[1] == data_Idx1[1] )
      {
         println( "prevIdx : " + data_Idx1[1] ) ;
         println( "backIdx : " + data_Idx2[1] ) ;
         println( "prevData : " + data_Idx1[0] ) ;
         println( "backData : " + data_Idx2[0]) ;
         println("success to hint via index") ;
      }
      else
      {
         println( "prevIdx : " + data_Idx1[1] ) ;
         println( "backIdx : " + data_Idx2[1] ) ;
         println( "prevData : " + data_Idx1[0] ) ;
         println( "backData : " + data_Idx2[0]) ;
         throw "failed to query by hint one." ;
      }
      var idxkey = JSON.parse( '{ "no" : -1, "score": 1 }' ) ;
      // Create Index and hint the exist index [T_Point2]
      createIndex( cl, indexName, idxkey, false, false ) ;
      hintInspectIndex( cl, indexName, "no", -1, false, false ) ;
      // Test Point : hint not exist index and no indexes in collection
      println( cl.find({no:{$lte:10000},score:{$gte:9999}}).hint({"":indexName}) ) ;
      var data_Idx3 = hintSnapshotSession( db, clName ) ;
      // The index will not be changed
      if ( data_Idx3[1] > data_Idx2[1] && data_Idx3[0] > data_Idx2[0] )
      {
         println( "prevIdx : " + data_Idx2[1] ) ;
         println( "backIdx : " + data_Idx3[1] ) ;
         println( "prevData : " + data_Idx2[0] ) ;
         println( "backData : " + data_Idx3[0]) ;
         println( "Success to hint via index" ) ;
      }
      else
      {
         println( "prevIdx : " + data_Idx2[1] ) ;
         println( "backIdx : " + data_Idx3[1] ) ;
         println( "prevData : " + data_Idx2[0] ) ;
         println( "backData : " + data_Idx3[0]) ;
         throw "Failed to query by hint two ." ;
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
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop collection in the end, wrong" ) ;
   db.close() ;
   throw e ;
}
