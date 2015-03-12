/******************************************************************************
@Description : Commom create index, and then check over.
@Modify list :
               2014-5-20  xiaojun Hu  Modify
*******************************************************************************/

function main( db )
{
   // drop collection
   commDropCL( db, csName, clName, true, true, "drop cl in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false, "create collection" ) ;

   // insert simple data to SDB
   idxCL.insert( {a:1} ) ;

   // create index and check
   createIndex( idxCL, "testindex", {a:1}, false) ;
   inspecIndex( idxCL, "testindex", "a", 1 ) ;

   // drop collectionspace in clean
   commDropCS( db, csName, true, "drop CS in clean " )
}

// main entry
try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}
