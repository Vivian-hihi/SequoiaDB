/************************************
*@Description: 创建固定集合空间集合，find查询数据 
*@author:      luweikang
*@createdate:  2017.7.12
*@testlinkCase:seqDB-11799
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
   var options = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false };
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
               {a:[10], b:2},
               {a:[102.03], b:2, c:"aaa"},
               {a:[1001], b:2, c:"aaa"},
               {a:["a"], b:2},
               {a:["z"], b:2, c:"aaa"},
               {b:1}];
   insertDate( dbcl, doc );
   
   //$in
   var inObj = { a: { $in: [ 10, "z"]}};
   var results1 = [  {a:10, b:1, c:"aaa"},
                     {a:[10], b:2},
                     {a:["z"], b:2, c:"aaa"}];
   checkResult( dbcl, inObj, results1 );
   
   //$ne
   var neObj = { b: { $ne: 1 }};
   var results2 = [  {a:"abc", b:2},
                     {a:{ MinKey:1 }, b:2, c:"aaa"},
                     {a:{ MaxKey:1 }, b:2, c:"aaa"},
                     {a:true, b:2},
                     {a:false, b:2, c:"aaa"},
                     {a:{name:"Jack"}, b:2, c:"aaa"},
                     {a:[10], b:2},
                     {a:[102.03], b:2, c:"aaa"},
                     {a:[1001], b:2, c:"aaa"},
                     {a:["a"], b:2},
                     {a:["z"], b:2, c:"aaa"}];
   checkResult( dbcl, neObj, results2 );
   
   //isNull
   var isnullObj = { c: {$isnull:1}};
   var results3 = [  {a:1001.02, b:1},
                     {a:{$date: "2017-05-19"}, b:1},
                     {a:{$regex:"^z",$options:"i"}, b:1},
                     {a:"abc", b:2},
                     {a:true, b:2},
                     {a:[10], b:2},
                     {a:["a"], b:2},
                     {b:1}]
   checkResult( dbcl, isnullObj, results3 );
   
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
                     {a:[102.03], b:2, c:"aaa"},
                     {a:[1001], b:2, c:"aaa"},
                     {a:["z"], b:2, c:"aaa"}];
   checkResult( dbcl, existsObj, results4 );
   
   //$add
   var addObj = {};
   
   //$include
   var includeObj = { a: { $include: 1} };
   var results01 =  [{a:10},
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
                     {a:[10]},
                     {a:[102.03]},
                     {a:[1001]},
                     {a:["a"]},
                     {a:["z"]},
                     {}];
   checkSelectResult( dbcl, includeObj, results01 );
   
   //insert 100000 record
   //insertRecord( dbcl, 100000 );
   //checkResultID( dbcl, 1, 16000 );
   //checkResultID( dbcl, 2, 32000 );
   //checkResultID( dbcl, 3, 48000 );
}

function checkResult( dbcl, options, results )
{
   
   var rc = dbcl.find( options );
   checkRec( rc, results );
   rc.close();
}

function checkSelectResult( dbcl, options, results )
{
   var rc = dbcl.find( null, options )
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

function insertRecord( dbcl, recordNum )
{
   var arr = [];
   for( var i = 0; i < 1024; i++ )
   {
      arr.push("a");
   }
   var str = arr.toString();
   var doc = [];
   for( var j = 0; j < recordNum; j++ )
   {
      doc.push({b:str});
   }
   try
   {
      dbcl.insert(doc);
   }
   catch( e )
   {
      throw buildException( "insertRecord()", e, "insert record", "insert success", "insert fail:"+e);
   }
}

function checkResultID( dbcl, fold, num )
{
   var rc = dbcl.find().sort( { _id: 1 } ).skip( num ).limit( 1 );
   var id = rc.next().toObj()._id;
   if(id < (33554396 * fold) )
   {
      throw "ERR_RECORD_ID";
   }
   rc.close();
}