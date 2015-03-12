/******************************************************************************
@Description : create index and specify the index field value not equal 1 or -1
@Modify list :
               2014-5-20  xiaojun Hu  Modify
******************************************************************************/

function main( db )
{
   // drop collection in the beginning
   commDropCL( db, csName, clName, true, true, "drop collection in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false, "create collection" ) ;

   // insert data to SDB
   idxCL.insert( {a:1} ) ;

   // create index
   createIndex( idxCL, "testindex", {a:10}, false, false, -6 ) ;

   // inspect the index
   try
   {
      inspecIndex( idxCL, "testindex", "a", 1, false ) ;
   }
   catch ( e )
   {
      if ( "ErrIdxName" != e )
      {
         throw e ;
      }
   }

   // drop collectionspace in clean
   commDropCS( db, csName, true, "drop CS in clean " )
}

//main entry
try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}
