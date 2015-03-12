/***************************************************************************
@Description : The Object have 5 layer nest, and the create composite index .
               such:[db.CS.CL.createIndex("arrLay5Index",
                    {"object1.object2.object3.object4.1":1,...}, true, true )]
@Modify list :
               2014-5-21  xiaojun Hu  Init
****************************************************************************/
function main( db )
{
   // drop collection in the beginning
   commDropCL( db, csName, clName, true, true, "drop collection in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false, "create collection" ) ;

   // insert data to SDB
   try
   {
      idxCL.insert({no:001,name:"A",age:2,
                   "object1":{"object2":{"object3":{"object4":{"object5":"5LayerObject"}}}}}) ;
      idxCL.insert({no:002,name:"a",age:3,"a1":{"a2":{"a3":{"a4":{"a5":"5LayerObject_a"}}}}}) ;
      idxCL.insert({no:003,name:"B",age:3,"b1":{"b2":{"b3":{"b4":{"b5":"5LayerObject_b"}}}}}) ;
      idxCL.insert({no:004,name:"C",age:3,"c1":{"c2":{"c3":{"c4":{"c5":"5LayerObject_c"}}}}}) ;
      idxCL.insert({no:005,name:"D",age:3,"d1":{"d2":{"d3":{"d4":{"d5":"5LayerObject_d"}}}}}) ;

      var i = 0 ;
      do
      {
         var count = idxCL.count() ;
         ++i ;
      }while( i < 10 )
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

   // create index
   createIndex( idxCL, "ObjLay5Index",
                {"object1.object2.object3.object4.object5":1,
                 "a1.a2.a3.a4.a5":-1, "b1.b2.b3.b4.b5":1,
                 "c1.c2.c3.c4.c5":1, "d1.d2.d3.d4.d5":-1}, true, true ) ;

   // inspect index
   try
   {
      inspecIndex( idxCL, "ObjLay5Index", "object1.object2.object3.object4.object5", 1, true, true ) ;
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

try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}
