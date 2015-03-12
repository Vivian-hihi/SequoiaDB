
// main entry
function main( db )
{
   // drop cl
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "drop cl in begin" ) ;
   var replSize = new RSize( COMMCSNAME ).ReplSize( db ) ;
   // create cl
   var varCL = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, {ShardingKey:{"no":1},ShardingType:"range",ReplSize:replSize,Compressed:true}, true, false, "create cl in begin" ) ;
   // create unique index
   commCreateIndex( varCL, "noIndex", {no:1}, true, false ) ;
   // check index
   commCheckIndex( varCL, "noIndex", true ) ;
   // insert data
   insertAndCheck( varCL, 20, false, true, "Insert not same data" ) ;
   // insert duplicate data
   var checkdup = false ;
   try
   {
      varCL.insert( {no:1} ) ;
   }
   catch( e )
   {
      if ( e == -38 )
      {
         checkdup = true ;
      }
      else
      {
         println( "Insert duplicate data failed: " + e ) ;
         throw e ;
      }
   }
   if ( !checkdup )
   {
      throw "index is unique, duplicate data should throw -38, but insert succeed" ;
   }

   // drop index
   commDropIndex( varCL, "noIndex", false ) ;
   // check not exist
   commCheckIndex( varCL, "noIndex", false ) ;

   // create un-unique index
   commCreateIndex( varCL, "noIndex", {no:1}, false, false ) ;
   // check index
   commCheckIndex( varCL, "noIndex", true ) ;
   // insert duplicate data
   try
   {
      varCL.insert( {no:1} ) ;
   }
   catch( e )
   {
      println( "Insert duplicate data failed: " + e ) ;
      throw e ;
   }
   // clean - drop cl
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false, "drop cl in clean" ) ;
}

try
{
   main( db ) ; 
   db.close() ;
}
catch( e )
{
   throw e ;
}
