/************************************
*@Description: seqDB-11353:seqDB-11353:rtnPredicate为[$minKey, $maxKey]的索引选择
*@author:      chimanzhao
*@createdate:  2020.4.25
*@testlinkCase: seqDB-11353
**************************************/

main();

function main()
{
   var clName = COMMCLNAME + "_11353";
   commDropCL( db, COMMCSNAME, clName, true );
   var dbcl   = commCreateCL( db, COMMCSNAME, clName );
   dbcl.insert( [{_id:1, a:1, b:2, c:3}, {_id:2, a:2, b:3, c:1}, {_id:3, a:3, b:1, c:2}] );
   dbcl.createIndex( "a",{a:1} );
   var flag   = 0;
   test( dbcl, flag );
   db.analyze();
   var flag   = 1;
   test( dbcl, flag );
   commDropCL( db, COMMCSNAME, clName );
}

function test( dbcl, flag )
{
	 
   var explainObj = dbcl.find( { b: 1 } ).explain().next().toObj();
   var find       = "find( { b: 1 } )";
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType; 
   check( "", IndexName,find, flag );
   check( "tbscan", ScanType,find, flag );

   
   var explainObj = dbcl.find( { b: 1 } ).hint( {"":"a"} ).explain().next().toObj();
   var find       = "find( { b: 1 } ).hint({\"\":\"a\"})";
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType; 
   check( "a", IndexName, find, flag );
   check( "ixscan", ScanType, find, flag );
  

   var explainObj = dbcl.find( { $or:[{a:2}, {c:1}] } ).explain().next().toObj();
   var find = "find( { $or:[{a:2},{c:1}] } )";
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType; 
   check( "", IndexName, find, flag );
   check( "tbscan", ScanType, find, flag );
   
   var explainObj = dbcl.find( { $or:[{a:2}, {c:1}] } ).hint( {"":"a"} ).explain().next().toObj();
   var find       = "find( { $or:[{a:2},{c:1}] } ).hint( {\"\":\"a\"} )"; 
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType; 
   check( "a", IndexName, find, flag );
   check( "ixscan", ScanType, find, flag );
   
   var explainObj = dbcl.find( { $not:[{a:2}, {c:1}] } ).explain().next().toObj();
   var find       = "find( { $not:[{a:2},{c:1}] } )";
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType; 
   check( "", IndexName, find, flag );
   check( "tbscan", ScanType, find, flag );
   
   var explainObj = dbcl.find( { $not:[{a:2}, {c:1}] } ).hint( {"":"a"} ).explain().next().toObj();
   var find       = "find( { $not:[{a:2},{c:1}] } ).hint( {\"\":\"a\"} )";   
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType; 
   check( "a", IndexName, find, flag );
   check( "ixscan", ScanType, find, flag );
}

function check( expect, actual , a, flag )
{
   if( expect !== actual )
   {
	  if( flag === 0 )
	  {
         println( a+" Error"+" before analyze()" );
	  }
	  else
	  {
	      println( a+" Error"+" after analyze()" );
	  }
      throw buildException( "check", "Error", check, expect, actual );
   }
}