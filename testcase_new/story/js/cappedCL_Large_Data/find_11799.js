/************************************
*@Description: 创建固定集合空间集合，find查询数据 
*@author:      luweikang
*@createdate:  2017.7.12
*@testlinkCase:seqDB-11799
**************************************/

main();

function main()
{
   var csName = CHANGEDPREFIX + "_11799_large_CS";
   var clName = CHANGEDPREFIX + "_11799_large_CL";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   //create cappedCL
   var options = { Capped : true, Size:1024, Max:10000000, AutoIndexId:false };
   var dbcl = commCreateCLByOption( db, csName, clName, options, false, false, "create capped cl" )
 
   //insert ond record at beginning
   var doc1 =  [{a:100},
               {a:101.01},
			   {a:"bbb"},
               {a:[10,20,30]}, 
			   {a:["c","d"]},
               {b:1}];
   insertDate( dbcl, doc1 );
 
   //insert 100000 record 
   insertRecord( dbcl, 100000 );
   
   //insert ond record at the end
   var doc2 =  [{a:200},
               {a:2002.02},
			   {a:"ddd"},
               {a:[100,200,300]}, 
			   {a:["x","y"]},
               {b:1}];
   insertDate( dbcl, doc2 );
   //check count
   checkCount( dbcl, {}, 100012 );
   
   //$in
   var inObj = { a: { $in: [ 10, 200, "c","x"]}};
   var results = [  {a:[10,20,30]}, 
			         {a:["c","d"]},
					 {a:200},
                     {a:[100,200,300]}, 
			         {a:["x","y"]}];
   checkFindResult( dbcl, inObj, results );
   checkCount( dbcl, inObj, 5 );
   
   //$gt
   var gtObj = { a: { $gt: 100}};
   var results = [{a:101.01}, 
			      {a:200},
                  {a:2002.02},
			      {a:[100,200,300]}];           
   checkFindResult( dbcl, gtObj, results );
   checkCount( dbcl, gtObj, 4 );
   
   checkResultID( dbcl, 1, 16000 );
   checkResultID( dbcl, 2, 32000 );
   checkResultID( dbcl, 3, 48000 );
   
   //clean environment after test  
   println( "---end the test---" );
  // commDropCS( db, csName, true, "drop CS in the end" );
}

function checkFindResult( dbcl, options, results )
{
   try
   {
      var rc = dbcl.find( options ).sort( { _id: 1 } );
   }
   catch( e )
   {
      throw buildException( "checkFindResult()", e, "find record", "find success", "find fail:"+e);
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
      doc.push({a:str});
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