/************************************
*@Description: decimal data use $cast
*@author:      zhaoyu
*@createdate:  2016.5.21
**************************************/
function main()
{
   //clean environment before test
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,"drop CL in the beginning" ) ;
   
   //create cl
   var dbcl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0 );
  
   //insert all kind of data include decimal
	var doc1 = [{a:-2147483648},
	            {a:2147483647},
	            {a:{$numberLong:"-9223372036854775808"}},
	            {a:{$numberLong:"9223372036854775807"}},
	            {a:-1.7E+308},
	            {a:1.7E+308},
	            {a:-4.9E-324},
	            {a:4.9E-324},
	            {a:"string"},
	            {a:{$oid:"573920accc332f037c000013"}},
	            {a:false},
	            {a:true},
	            {a:{$date:"2016-05-16"}},
	            {a:{$timestamp:"2016-05-16-13.14.26.124233"}},
	            {a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}},
	            {a:{$regex:"^z",$options:"i"}},
	            {a:{name:"hanmeimei"}},
	            {a:["b",0]},
	            {a:null}];
	insertData(dbcl, doc1);
	
	//cast decimal
	var expRecs1 = [{a:{$decimal:"-2147483648"}},
	                {a:{$decimal:"2147483647"}},
	                {a:{$decimal:"-9223372036854775808"}},
	                {a:{$decimal:"9223372036854775807"}},
	                {a:{$decimal:"-170000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"}},
	                {a:{$decimal:"170000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"}},
	                {a:{$decimal:"-0.00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000494065645841247"}},
	                {a:{$decimal:"0.00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000494065645841247"}},
	                {a:{$decimal:"0"}},
	                {a:{$decimal:"0"}},
	                {a:{$decimal:"0"}},
	                {a:{$decimal:"1"}},
	                {a:{$decimal:"1463328000000"}},
	                {a:{$decimal:"1463375666124"}},
	                {a:{$decimal:"0"}},
	                {a:{$decimal:"0"}},
	                {a:{$decimal:"0"}},
	                {a:[{$decimal:"0"},{$decimal:"0"}]},
	                {a:{$decimal:"0"}}];
	checkResult( dbcl, null,{a:{$cast:100}}, expRecs1, {_id:1} );
	//delete data
	deleteData( dbcl, null );
	
	//insert decimal data
	var doc2 = [{a:{$decimal:"123"}},{a:{$decimal:"-9223372036854775809"}},{a:{$decimal:"9223372036854775808",$precision:[100,2]}}];
	insertData(dbcl, doc2);
	
	//cast int
	var expRecs2 = [{a:123},{a:0},{a:0}];
	checkResult( dbcl, null,{a:{$cast:"int32"}}, expRecs2, {_id:1} );
	
	//delete data
	deleteData( dbcl, null );
	
	//insert decimal data
	var doc3 = [{a:{$decimal:"9223372036854775807"}},{a:{$decimal:"-9223372036854775809"}},{a:{$decimal:"9223372036854775808",$precision:[100,2]}}];
	insertData(dbcl, doc3);
	
	//cast numberLong
	var expRecs4 = [{a:{$numberLong:"9223372036854775807"}},{a:0},{a:0}];
	checkResult( dbcl, null,{a:{$cast:"int64"}}, expRecs4, {_id:1} );
	
	//delete data
	deleteData( dbcl, null );
	
	//insert decimal data
	var doc5 = [{a:{$decimal:"-1.7E+308"}},{a:{$decimal:"1.7E+310"}},{a:{$decimal:"1.7E+310",$precision:[1000,2]}}];
	insertData(dbcl, doc5);
	
	//cast double
   var expRecs5 = [{a:-1.7E+308},{a:0},{a:0}];
	checkResult( dbcl, null,{a:{$cast:1}}, expRecs5, {_id:1} );
}

main();