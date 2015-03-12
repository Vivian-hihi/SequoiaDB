/***************************************************************************
@Description :First create index , and then insert data. The index have
              unique and enforced arguments. enforced unique index.
@Modify list :
              2014-5-15  xiaojun Hu  Create
****************************************************************************/
function main( db )
{
   // drop collection in the beginning
   commDropCL( db, csName, clName, true, true, "drop collection in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false, "create collection" ) ;

   // insert data first
   try
   {
      idxCL.insert({no:001,name:"A",age:1}) ;
      idxCL.insert({no:002,name:"B",age:2}) ;
      idxCL.insert({no:003,name:"C","姓名":"张"}) ;
      idxCL.insert({no:004,name:"C","姓名":"庄"}) ;
   }
   catch ( e )
   {
      println( "Failed to insert record, rc= " +e ) ;
      throw e ;
   }

   // create index
   createIndex( idxCL, "noIndex", {no:1}, true, true ) ;
   createIndex( idxCL, "nameIndex", {name:-1}, true, true, -38 ) ;
   createIndex( idxCL, "姓名索引", {"姓名":1}, true, true, -38 ) ;
   createIndex( idxCL, "ageIndex", {"age":1}, true, true, -38 ) ;

   // inspect the index
   try
   {
      inspecIndex( idxCL, "noIndex", "no", 1, true, true ) ;
      inspecIndex( idxCL, "nameIndex", "name", -1, true, true ) ;
      inspecIndex( idxCL, "姓名索引", "姓名", 1, true, true ) ;
      inspecIndex( idxCL, "ageIndex", "age", 1, true, true ) ;
      println( "Can go end of program." ) ;
   }
   catch ( e )
   {
      if ( "ErrIdxName" != e )
      {
         throw e ;
      }
   }

   //insert data after create index
   try
   {
      idxCL.insert({no:011,name:"A","姓名":"李",age:1}) ;
      idxCL.insert({no:022,name:"B","姓名":"王",age:2}) ;
      idxCL.insert({no:033,name:"C","姓名":"张",age:3}) ;
      idxCL.insert({no:044,name:"B","姓名":"庄",age:5}) ;
      idxCL.insert({no:055,name:"E","姓名":"汉",age:8}) ;
      var count = idxCL.count() ;
      if ( 9 != count )
      {
         println( "Wrong number of record :"+count ) ;
         throw "ErrNumRecord" ;
      }
   }
   catch ( e )
   {
      println( "Failed to insert date after create index : "+e ) ;
      throw e ;
   }

   // drop collectionspace in clean
   commDropCS( db, csName, true, "drop CS in clean " )
}

try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}

