/*******************************************************************************
@Description : Query matcher $and/$or/$not test.
@Modify list :
               2014-10-10  xiaojun Hu  changed
*******************************************************************************/
function main( db )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clear collection in the beginning" ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "create collection in the beginning" ) ;
   // insert data
   insertData( db, COMMCSNAME, COMMCLNAME ) ;
   // matcher $and
   try
   {
      var query = cl.find( {$and: [{interest: ["football", "basketball",
                                  "pingpong"]}, {"CustomerNumber":100009}]} ) ;
      if( 1 != query.count() )
      {
         println( "query data in $and = " + query.count() ) ;
         throw "query wrong number of reacord" ;
      }
   }
   catch( e )
   {
      println( "failed to query data, rc = " + e ) ;
      throw e ;
   }
   // matcher $or
   try
   {
      //var query1 = cl.find( {$or: [{interest: ["football", "basketball",
     //                           "pingpong"]}, {"CustomerNumber":100009}]} ) ;
      var query1 = cl.find( {interest: ["football", "basketball", "pingpong"]} ) ;
      if( 100 != query1.count() )
      {
         println( "query data in $or = " + query1.count() ) ;
         throw "query wrong number of reacord" ;
      }
   }
   catch( e )
   {
      println( "failed to query data, rc = " + e ) ;
      throw e ;
   }
   // matcher $not
   try
   {
      var query2 = cl.find({$not:[{"CustomerNumber":100009}]}) ;
      if( 99 != query2.count() )
      {
         println( "query data in $not = " + query2.count() ) ;
         throw "query wrong number of reacord" ;
      }
   }
   catch( e )
   {
      println( "failed to query data, rc = " + e ) ;
      throw e ;
   }
   // matcher $and/$not/$or
   try
   {
      var query3 = cl.find({$and:[{$or:[{"CustomerNumber":100009},
                                        {"CustomerNumber":100090}]}]}) ;
      if( 2 != query3.count() )
      {
         println( "query data in $and/$or = " + query3.count() ) ;
         throw "query wrong number of reacord" ;
      }
   }
   catch( e )
   {
      println( "failed to query data, rc = " + e ) ;
      throw e ;
   }
   println( "success to query matcher $and/$or/$not" ) ;
   // clear collection space in the end
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "clear collection in the end" ) ;
}

// Run Main
try
{
   main( db ) ;
}
catch( e )
{
   throw e ;
}
