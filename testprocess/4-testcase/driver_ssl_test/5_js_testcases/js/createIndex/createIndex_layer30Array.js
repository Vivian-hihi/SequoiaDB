/***************************************************************************
@Description : The array have 35 layer nest, and the create index .
               such:[db.CS.CL.createIndex("arrLay5Index",
                    {"array1.array2.array3.array4.1":1}, true, true )]
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
      idxCL.insert({no:001,name:"A",age:2,"array1":[{"array2":[{"array3":[{"array4":[{"array5":[{"array6":[{"array7":[{"array8":[{"array9":[{"array10":[{"array11":[{"array12":[{"array13":[{"array14":[{"array15":[{"array16":[{"array17":[{"array18":[{"array19":[{"array20":[{"array21":[{"array22":[{"array23":[{"array24":[{"array25":[{"array26":[{"array27":[{"array28":[{"array29":["array30","TEMP30"]},"temp28"]},"temp27"]},"temp26"]},"temp25"]},"temp24"]},"temp23"]},"temp22"]},"temp21"]},"temp20"]},"temp19"]},"temp18"]},"temp17"]},"temp16"]},"temp15"]},"temp14"]},"temp13"]},"temp12"]},"temp11"]},"temp10"]},"temp9"]},"temp8"]},"temp7"]},"temp6"]},"temp5"]},"temp4"]},"temp3"]},"temp2"]},"temp1"]}) ;
      var i = 0 ;
      do
      {
         var count = idxCL.count() ;
         ++i ;
      }while( i < 10 )
      if ( 1 != count)
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

   // create index
   createIndex( idxCL, "arrLay30Index", {"array1.array2.array3.array4.array5.array6.array7.array8.array9.array10.array11.array12.array13.array14.array15.array16.array17.array18.array19.array20.array21.array22.array23.array24.array25.array26.array27.array28.array29":1}, true, true ) ;

   // inspect index
   try
   {
      inspecIndex( idxCL, "arrLay30Index","array1.array2.array3.array4.array5.array6.array7.array8.array9.array10.array11.array12.array13.array14.array15.array16.array17.array18.array19.array20.array21.array22.array23.array24.array25.array26.array27.array28.array29", 1, true, true ) ;
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
