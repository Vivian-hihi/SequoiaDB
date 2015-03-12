/*******************************************************************************
*@Description : Test aggregate method using complex combination of argument
*               options.
*@Modify list :
*               2014-10-10  xiaojun Hu  change
*******************************************************************************/

function main( db )
{
   // Drop collection in the beginning
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to clean environment in the beginning" ) ;
   // Create Collection and auto specify CollectionSpaces
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true,
                          false, "Failed to create collection" ) ;
   // Insert data to SDB
   for( var i =0; i < 1000; i++ )
   {
      cl.insert({ no:i, price:i*10, group:i%4 }) ;
   }
   var cnt = 0 ;
   do
   {
      count = cl.count() ;
      ++cnt ;
      if( 500 == cnt )
      {
         println( "recore should be 1000, count = " + count ) ;
         throw "error_count" ;
      }
   }while( 1000 != count ) ;

   try
   {
      rc = cl.aggregate( {$match:{$and:[{no:{$gte:10}},{group:{$in:[1,2,3]}},
                         {no:{$mod:[5,3]}}]}}, {$sort:{group:1,no:1}},
                         {$match:{$or:[{price:{$lt:1500,$gt:1000}},
                         {price:{$lt:4000,$gt:3500}},{price:{$lt:9990,$gt:9000}}]}},
                         {$skip:5}, {$limit:100},
                         {$group: {_id:"$group", avg_price:{$avg:"$price"},
                                   sum_price:{$sum:"$price"}, max_price:{$max:"$price"},
                                   min_price:{$min:"$price"}, group:{$first:"$group"},
                                   push_price:{$push:"$price"} }
                          } ) ;
   }
   catch( e )
   {
      println("aggregate failed, rc = " + rc ) ;
      throw e ;
   }

   var i =0 ;
   while( rc.next() )
   {
      i++ ;
      var recordJson = rc.current().toJson() ;
      var obj = eval("("+recordJson+")") ;
      var fieldarray = new Array() ;
      for( var fieldname in obj )
      {
         fieldarray.push( fieldname ) ;
      }
      if( !((fieldarray.length == 6) && (fieldarray[0] == "avg_price" &&
           fieldarray[1] == "sum_price" && fieldarray[2] == "max_price" &&
           fieldarray[3] == "min_price" && fieldarray[4] == "group" &&
           fieldarray[5] == "push_price" )))
      {
         //println("return wrong field names");
         for(var j=0;j<fieldarray.length;j++)
         {
            println("fieldarray["+j+"]="+fieldarray[j]);	
         }
         throw "return wrong field names" ;
      }
   }

   if( i != 3 )
   {
      println("return wrong numbers of records, i = " + i ) ;
      println( cl.aggregate( {$match:{$and:[{no:{$gte:10}}, {group:{$in:[1,2,3]}},
                             {no:{$mod:[5,3]}}]}}, {$sort:{group:1,no:1}},
                             {$match:{$or:[{price:{$lt:1500,$gt:1000}},
                             {price:{$lt:4000,$gt:3500}},
                             {price:{$lt:9990,$gt:9000}}]}}, {$skip:5}, {$limit:100},
                             {$group: {_id:"$group", avg_price:{$avg:"$price"},
                             sum_price:{$sum:"$price"}, max_price:{$max:"$price"},
                             min_price:{$min:"$price"}, group:{$first:"$group"},
                             push_price:{$push:"$price"} }
                             }));
      throw "return wrong numbers" ;
   }
   // Clear environment in the end
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to clean environment in the end" ) ;
}

// Run Main
try
{
   main( db ) ;
}
catch (e)
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to clean environment in the end" ) ;
   throw e ;
}

