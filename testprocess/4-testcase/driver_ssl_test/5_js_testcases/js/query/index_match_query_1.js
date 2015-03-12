/*******************************************************************************
*@Description : create index and query. query match $gt/$gte/$lt/$lte/$ne
*                                                   $and/$not/$or
*@Modify list :
*               2014-5-20  xiaojun Hu  Init
*******************************************************************************/

function main()
{
   var insertNum = 10000 ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "create collection in the beginning" ) ;
   // insert data
   idxAutoGenData( cl, insertNum ) ;
   println( "insert " + insertNum + " data successful" ) ;

   // create Index
   var idxName = "noIndex" ;
   var indexDef = { "no":1, "no1":1, "no2":-1, "no3":1 } ;
   commCreateIndex( cl, idxName, indexDef, false, false ) ;
   commCheckIndex( cl, idxName, true ) ;
   println( "create index successful" ) ;

   // query data
   var queryCond = {$and:[{"no":{$gt:insertNum/100}}, {"no1":{$gte:insertNum*2/100+2}},
                          {"no2":{$lt:insertNum*3/100+10}},
                          {"array":{"$in":[ "5arr5", 5 ]}},
                          {"no3":{$lte:insertNum*4/100+8}}]} ;
   println( "" ) ;
   println( "Condition: " + JSON.stringify(queryCond) ) ;
   idxQueryCheck( cl, queryCond, 0, idxName ) ;
   var queryCond = {$and:[{"no":{$gt:insertNum/100}}, {"no1":{$gte:insertNum*2/100+2}},
                          {"no2":{$lt:insertNum*3/100+10}},
                          {"array":{"$in":[ "102arr102", 102 ]}},
                          {"no3":{$lte:insertNum*4/100+8}}]} ;
   println( "" ) ;
   println( "Condition: " + JSON.stringify(queryCond) ) ;
   idxQueryCheck( cl, queryCond, 1, idxName ) ;
   var queryCond = {$and:[{"no":{$gt:insertNum/100}}, {"no1":{$gte:insertNum*2/100+2}},
                          {"no2":{$lt:insertNum*3/100+10}},
                          {"array":{"$nin":[ "102arr102", 102 ]}},
                          {"no3":{$lte:insertNum*4/100+8}}]} ;
   println( "" ) ;
   println( "Condition: " + JSON.stringify(queryCond) ) ;
   idxQueryCheck( cl, queryCond, 1, idxName ) ;
   var queryCond = {$not:[{"no":{$gt:insertNum/100}}, {"no1":{$gte:insertNum*2/100+2}},
                          {"no":{"$ne":102}},
                          {"no2":{$lt:insertNum*3/100+10}},
                          {"no3":{$lte:insertNum*4/100+8}}]} ;
   println( "" ) ;
   println( "Condition: " + JSON.stringify(queryCond) ) ;
   idxQueryCheck( cl, queryCond, 9999, idxName ) ;
   var queryCond = {$or:[{$and:[{"no":{$gt:insertNum/100}},
                         {"no":{$lt:insertNum/100+10}}]},
                         {"array":{"$all":["9999arr9999", 49995, "19998ARR9999",
                                           "arrayIndex"]}},
                         {$and:[{"no1":{$gte:insertNum*2/100}},
                         {"no1":{$lte:insertNum*2/100+8}}]}]} ;
   println( "" ) ;
   println( "Condition: " + JSON.stringify(queryCond) ) ;
   idxQueryCheck( cl, queryCond, 11, idxName ) ;

}

// Run Main

try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to drop collection in the begin" ) ;
   main( db ) ;
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
   //            "failed to drop collection in the end, correct" ) ;
   db.close() ;
}
catch( e )
{
   //commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
   //            "failed to drop collection in the end, wrong" ) ;
   db.close() ;
   throw e ;
}
