/*******************************************************************************
*@Description : test function : db.foo.bar.findOne().
*@Modify List :
*               2014-9-26   xiaojunHu  Init
*******************************************************************************/

function main( db )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to drop cl in the begnning" ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "failed to create collection in the beginning" ) ;
   for( var i = 0 ; i < 1000 ; ++i )
   {
      cl.insert( { no:i, number:2000-i, id:3*i, description: +
                   "findOne() equel find().limit(1), flag = " +
                   "FLG_QUERY_WITH_RETURNDATA" } ) ;
   }
   var cnt = 0 ;
   var count = 0 ;
   println( "success to insert data" ) ;
   do
   {
      count = cl.count() ;
      ++cnt ;
   }while( 1000 != count || cnt < 100 ) ;
   if( 1000 != count )
   {
      println( "insert records number = " + count ) ;
      throw "insert wrong records" ;
   }
   println( "success to count" ) ;
   // findOne no options
   try
   {
      var query = cl.findOne() ;
      var count = query.toArray() ;
      if( 1 != count.length )
      {
         println( query ) ;
         throw "get wrong return record" ;
      }
   }
   catch( e )
   {
      println( "failed to findOne, rc = " + e ) ;
      throw e ;
   }
   // findOne and options query one
   try
   {
      var query = cl.findOne( {no:500} ).toArray() ;
      var queryObj = eval( "(" + query + ")" ) ;
      if( 500 != queryObj.no )
      {
         println( query ) ;
         throw "get wrong return record" ;
      }
   }
   catch( e )
   {
      println( "failed to findOne and specify one, rc = " + e ) ;
      throw e ;
   }
   // findOne and options query more than one
   try
   {
      var query = cl.findOne( { id: {$lt:10} } ).toArray() ;
      if( 1 != query.length )
      {
         println( query ) ;
         throw " get wrong return record" ;
      }
   }
   catch( e )
   {
      println( "failed to findOne and specify more than one, rc = " + e ) ;
      throw e ;
   }
   println( "success to run findOne" ) ;
   // clear in the end
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to drop cl in the end" ) ;
}

//Main Running
try
{
   main( db ) ;
}
catch( e )
{
   throw e ;
}
