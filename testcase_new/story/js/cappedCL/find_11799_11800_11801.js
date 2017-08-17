/************************************
*@Description: 创建固定集合空间集合，find查询数据 
*@author:      luweikang
*@createdate:  2017.7.12
*@testlinkCase:seqDB-11799,seqDB-11800,seqDB-11801
**************************************/

main();

function main()
{
   var csName = CHANGEDPREFIX + "_11799_CS";
   var clName = CHANGEDPREFIX + "_11799_CL";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   //create cappedCL
   var options = { Capped : true, Size:1024, Max:10000000, AutoIndexId:false };
   var dbcl = commCreateCLByOption( db, csName, clName, options, false, false, "create capped cl" )
   
   //insert data 
   var doc =  [{a:10, b:1, c:"aaa"},
               {a:100, b:1, c:"aaa"},
               {a:1001.02, b:1},
               {a:{$decimal: "20170519.09"}, b:1, c:"aaa"}, 
               {a:{$numberLong:"9223372036854775807"}, b:1, c:"aaa"},
               {a:{$date: "2017-05-19"}, b:1},
               {a:{$timestamp: "2017-05-19-15.32.18.000000"}, b:1, c:"aaa"},
               {a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}, b:1, c:"aaa"},
               {a:{$regex:"^z",$options:"i"}, b:1},
               {a:null, b:1, c:"aaa"},
               {a:{$oid:"123abcd00ef12358902300ef"}, b:1, c:"aaa"}, 
               {a:"abc", b:2},
               {a:{ MinKey:1 }, b:2, c:"aaa"},
               {a:{ MaxKey:1 }, b:2, c:"aaa"},
               {a:true, b:2},
               {a:false, b:2, c:"aaa"},
               {a:{name:"Jack"}, b:2, c:"aaa"},
               {a:[10,11,12], b:2},
               {a:[102.03,103.4,104.5], b:2, c:"aaa"},
               {a:[1001], b:2, c:"aaa"},
               {a:["a","b","c"], b:2},
               {a:["z"], b:2, c:"aaa"},
               {b:1}];
   insertDate( dbcl, doc );
   
   //check count
   checkCountResult( dbcl, {}, 23 );
   
   //$in
   var inObj = { a: { $in: [ 10, "z"]}};
   var results1 = [  {a:10, b:1, c:"aaa"},
                     {a:[10,11,12], b:2},
                     {a:["z"], b:2, c:"aaa"}];
   checkFindResult( dbcl, inObj, results1 );
   checkCountResult( dbcl, inObj, 3 );
   
   //$ne
   var neObj = { b: { $ne: 1 }};
   var results2 = [  {a:"abc", b:2},
                     {a:{ MinKey:1 }, b:2, c:"aaa"},
                     {a:{ MaxKey:1 }, b:2, c:"aaa"},
                     {a:true, b:2},
                     {a:false, b:2, c:"aaa"},
                     {a:{name:"Jack"}, b:2, c:"aaa"},
                     {a:[10,11,12], b:2},
                     {a:[102.03,103.4,104.5], b:2, c:"aaa"},
                     {a:[1001], b:2, c:"aaa"},
                     {a:["a","b","c"], b:2},
                     {a:["z"], b:2, c:"aaa"}];
   checkFindResult( dbcl, neObj, results2 );
   checkCountResult( dbcl, neObj, 11 );
   
   //isNull
   var isnullObj = { c: {$isnull:1}};
   var results3 = [  {a:1001.02, b:1},
                     {a:{$date: "2017-05-19"}, b:1},
                     {a:{$regex:"^z",$options:"i"}, b:1},
                     {a:"abc", b:2},
                     {a:true, b:2},
                     {a:[10,11,12], b:2},
                     {a:["a","b","c"], b:2},
                     {b:1}]
   checkFindResult( dbcl, isnullObj, results3 );
   checkCountResult( dbcl, isnullObj, 8 );
   
   //$exists
   var existsObj = { c: {$exists: 1}};
   var results4 = [  {a:10, b:1, c:"aaa"},
                     {a:100, b:1, c:"aaa"},
                     {a:{$decimal: "20170519.09"}, b:1, c:"aaa"}, 
                     {a:{$numberLong:"9223372036854775807"}, b:1, c:"aaa"},
                     {a:{$timestamp: "2017-05-19-15.32.18.000000"}, b:1, c:"aaa"},
                     {a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}, b:1, c:"aaa"},
                     {a:null, b:1, c:"aaa"},
                     {a:{$oid:"123abcd00ef12358902300ef"}, b:1, c:"aaa"}, 
                     {a:{ MinKey:1 }, b:2, c:"aaa"},
                     {a:{ MaxKey:1 }, b:2, c:"aaa"},
                     {a:false, b:2, c:"aaa"},
                     {a:{name:"Jack"}, b:2, c:"aaa"},
                     {a:[102.03,103.4,104.5], b:2, c:"aaa"},
                     {a:[1001], b:2, c:"aaa"},
                     {a:["z"], b:2, c:"aaa"}];
   checkFindResult( dbcl, existsObj, results4 );
   checkCountResult( dbcl, existsObj, 15 );
   
   //$add
   var addObj = { b: { "$add": 10} };
   var results5 = [  {a:10, b:11, c:"aaa"},
                     {a:100, b:11, c:"aaa"},
                     {a:1001.02, b:11},
                     {a:{$decimal: "20170519.09"}, b:11, c:"aaa"}, 
                     {a:{$numberLong:"9223372036854775807"}, b:11, c:"aaa"},
                     {a:{$date: "2017-05-19"}, b:11},
                     {a:{$timestamp: "2017-05-19-15.32.18.000000"}, b:11, c:"aaa"},
                     {a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}, b:11, c:"aaa"},
                     {a:{$regex:"^z",$options:"i"}, b:11},
                     {a:null, b:11, c:"aaa"},
                     {a:{$oid:"123abcd00ef12358902300ef"}, b:11, c:"aaa"}, 
                     {a:"abc", b:12},
                     {a:{ MinKey:1 }, b:12, c:"aaa"},
                     {a:{ MaxKey:1 }, b:12, c:"aaa"},
                     {a:true, b:12},
                     {a:false, b:12, c:"aaa"},
                     {a:{name:"Jack"}, b:12, c:"aaa"},
                     {a:[10,11,12], b:12},
                     {a:[102.03,103.4,104.5], b:12, c:"aaa"},
                     {a:[1001], b:12, c:"aaa"},
                     {a:["a","b","c"], b:12},
                     {a:["z"], b:12, c:"aaa"},
                     {b:11}];
   checkSelectResult( dbcl, addObj, results5 );
   
   //$expand
   var expandObj = {a: {$expand: 1}}
   var results6 = [  {a:10, b:1, c:"aaa"},
                     {a:100, b:1, c:"aaa"},
                     {a:1001.02, b:1},
                     {a:{$decimal: "20170519.09"}, b:1, c:"aaa"}, 
                     {a:{$numberLong:"9223372036854775807"}, b:1, c:"aaa"},
                     {a:{$date: "2017-05-19"}, b:1},
                     {a:{$timestamp: "2017-05-19-15.32.18.000000"}, b:1, c:"aaa"},
                     {a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}, b:1, c:"aaa"},
                     {a:{$regex:"^z",$options:"i"}, b:1},
                     {a:null, b:1, c:"aaa"},
                     {a:{$oid:"123abcd00ef12358902300ef"}, b:1, c:"aaa"}, 
                     {a:"abc", b:2},
                     {a:{ MinKey:1 }, b:2, c:"aaa"},
                     {a:{ MaxKey:1 }, b:2, c:"aaa"},
                     {a:true, b:2},
                     {a:false, b:2, c:"aaa"},
                     {a:{name:"Jack"}, b:2, c:"aaa"},
                     {a:10, b:2},
                     {a:11, b:2},
                     {a:12, b:2},
                     {a:102.03, b:2, c:"aaa"},
                     {a:103.4, b:2, c:"aaa"},
                     {a:104.5, b:2, c:"aaa"},
                     {a:1001, b:2, c:"aaa"},
                     {a:"a", b:2},
                     {a:"b", b:2},
                     {a:"c", b:2},
                     {a:"z", b:2, c:"aaa"},
                     {b:1}];
   checkFindResult( dbcl, expandObj, results6 );
   
   //$include
   var includeObj = { a: { $include: 1} };
   var results7 =  [{a:10},
                     {a:100},
                     {a:1001.02},
                     {a:{$decimal: "20170519.09"}}, 
                     {a:{$numberLong:"9223372036854775807"}},
                     {a:{$date: "2017-05-19"}},
                     {a:{$timestamp: "2017-05-19-15.32.18.000000"}},
                     {a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}},
                     {a:{$regex:"^z",$options:"i"}},
                     {a:null},
                     {a:{$oid:"123abcd00ef12358902300ef"}}, 
                     {a:"abc"},
                     {a:{ MinKey:1 }},
                     {a:{ MaxKey:1 }},
                     {a:true},
                     {a:false},
                     {a:{name:"Jack"}},
                     {a:[10,11,12]},
                     {a:[102.03,103.4,104.5]},
                     {a:[1001]},
                     {a:["a","b","c"]},
                     {a:["z"]},
                     {}];
   checkSelectResult( dbcl, includeObj, results7 );
   
   //findOne
   var findObj = {};
   var results8 = {a:10, b:1, c:"aaa"};
   checkFindOneResult( dbcl, findObj, results8 );
   
   //findOne $et
   var etObj = { a: {$et: "a"}};
   var results9 = {a:["a","b","c"], b:2};
   checkFindOneResult( dbcl, etObj, results9 );
   
   //clean environment after test  
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkFindResult( dbcl, options, results )
{
   try
   {
      var rc = dbcl.find( options );
   }
   catch( e )
   {
      throw buildException( "checkFindResult()", e, "find record", "find success", "find fail:"+e);
   }
   checkRec( rc, results );
   rc.close();
}

function checkCountResult( dbcl, options, results )
{
   try
   {
      var exc = dbcl.count( options )
   }
   catch( e )
   {
      throw buildException( "checkCountResult()", e, "count record", "count success", "count fail:"+e);
   }
   if( Number(exc) !== results )
   {
      throw "ERR_COUNT_NUM";
   }
}

function checkFindOneResult( dbcl, options, results )
{
   try
   {
      var rc = dbcl.findOne( options );
   }
   catch( e )
   {
      throw buildException( "checkFindOneResult()", e, "find record", "find success", "find fail:"+e);
   }
   var obj = rc.current().toObj();
   var id = obj._id;
   if( id === undefined )
   {
      throw "ERR_VALUE_ID";
   }
   for( var i in obj )
   {
      if(i == "_id")
      {
         continue;
      }
      if( JSON.stringify( obj[i] ) !== JSON.stringify( results[i] ) )
      {
         throw buildException( "checkFindOneResult()", null, "find record", JSON.stringify( obj ), JSON.stringify( results ));
      }
   }
}

function checkSelectResult( dbcl, options, results )
{
   try
   {
      var rc = dbcl.find( null, options )
   }
   catch( e )
   {
      throw buildException( "checkSelectResult()", e, "find record", "find success", "find fail:"+e);
   }
   checkRec( rc, results );
   rc.close();
}

function insertDate( dbcl, doc )
{
   try
   {
      dbcl.insert( doc );
   }
   catch( e )
   {
      throw buildException( "insertDate()", e, "insert record", "insert success", "insert fail:"+e);
   }
}


