/************************************
*@Description: type as selector
*@author:      zhaoyu
*@createdate:  2016.11.1
*@testlinkCase: 
**************************************/
function main()
{
   //clean environment before test
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,"drop CL in the beginning" ) ;
   
   //create cl
   var dbcl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0);
   
   //insert data 
	var doc = [{No:1,a:123,b:{$numberLong:"9223372036854775807"},c:123.123,d:{$decimal:"123",$precision:[5,2]}},
	           {No:2,a:"string",b:{$oid:"123abcd00ef12358902300ef"},c:true,d:{$date:"2012-12-12"}},
	           {No:3,a:{$timestamp:"2012-01-01-13.14.26.124233"},c:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"},d:{$regex:'^string$',$options:'i'}},
	           {No:4,a:{0:1},b:[1,2,3],c:null},
	           {No:5,b:MinKey(),c:MaxKey()}];
	insertData(dbcl, doc);
   
   //many field,some exists,some Non-exists
   var selector1 = {a:{$type:1},b:{$type:1},c:{$type:1},d:{$type:1}};
   var expRecs1 = [{No:1,a:16,b:18,c:1,d:100},
	                {No:2,a:2,b:7,c:8,d:9},
	                {No:3,a:17,c:5,d:11},
	                {No:4,a:3,b:4,c:10},
	                {No:5,b:-1,c:127}];
   checkResult( dbcl, null, selector1, expRecs1, {No:1} );
   
   var selector2 = {a:{$type:2},b:{$type:2},c:{$type:2},d:{$type:2}};
   var expRecs2 = [{No:1,a:"int32",b:"int64",c:"double",d:"decimal"},
	                {No:2,a:"string",b:"oid",c:"bool",d:"date"},
	                {No:3,a:"timestamp",c:"bindata",d:"regex"},
	                {No:4,a:"object",b:"array",c:"null"},
	                {No:5,b:"minkey",c:"maxkey"}];
   checkResult( dbcl, null, selector2, expRecs2, {No:1} );
   
   var selector3 = {a:{$type:"a"}};
   InvalidArgCheck( dbcl, null, selector3, -6, {No:1} );
   
   
}
main()