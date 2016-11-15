/************************************
*@Description: slice as function
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
	var doc = [{No:1,a:[1,2,3]},
	           {No:2,a:[1,2]},
	           {No:3,a:[1,2,3,4,5]},
	           {No:4,a:[1,2,3,4]},
	           {No:5,a:[]},
	           {No:6,a:1}];
	insertData(dbcl, doc);
   
   //many field,some exists,some Non-exists,SEQUOIADBMAINSTREAM-2036
   var selector1 = {a:{$size:1},b:{$size:1}};
   var expRecs1 = [{No:1,a:3},
	                {No:2,a:2},
	                {No:3,a:5},
	                {No:4,a:4},
	                {No:5,a:0},
	                {No:6,a:null}];
   checkResult( dbcl, null, selector1, expRecs1, {No:1} );
   
   var selector2 = {a:{$type:"a"}};
   InvalidArgCheck( dbcl, null, selector2, -6, {No:1} );
   
   
}
main()