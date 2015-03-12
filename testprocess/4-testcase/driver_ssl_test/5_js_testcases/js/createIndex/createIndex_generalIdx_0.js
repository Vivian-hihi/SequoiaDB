/***************************************************************************
@Description :First create index, and then insert data. The index don't have
              unique and enforced arguments.
@Modify list :
              2014-5-15  xiaojun Hu  Create
****************************************************************************/
function main( db )
{
   // drop collection in the beginning
   commDropCL( db, csName, clName, true, true, "drop collection in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false, "create collection" ) ;

   // create index
   createIndex( idxCL, "noIndex", {no:1}, false, false ) ;
   createIndex( idxCL, "nameIndex", {name:-1}, false, false ) ;
   createIndex( idxCL, "姓名索引", {"姓名":1}, false, false ) ;
   createIndex( idxCL, "ageIndex", {"age":1}, false, false ) ;
   // inspect the index
   try
   {
      inspecIndex( idxCL, "noIndex", "no", 1, false, false ) ;
      inspecIndex( idxCL, "nameIndex", "name", -1, false, false ) ;
      inspecIndex( idxCL, "姓名索引", "姓名", 1, false, false ) ;
      inspecIndex( idxCL, "ageIndex", "age", 1, false, false ) ;
   }
   catch ( e )
   {
      if ( "ErrIdxName" != e )
      {
         throw e ;
      }
   }

   //after create index, insert data
   try
   {
      idxCL.insert({"no1":1001,"coutry":"china","名字":"晨昴"}) ;
      idxCL.insert({no:001,name:"A",age:1}) ;
      idxCL.insert({no:002,name:"B",age:2}) ;
      idxCL.insert({no:003,name:"C","姓名":"张"}) ;
      idxCL.insert({no:004,name:"C","姓名":"庄"}) ;
      count=idxCL.count() ;
      if ( 5 != count)
      {
         println( "Wrong number of record :"+count ) ;
         throw "ErrNumRecord"
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
