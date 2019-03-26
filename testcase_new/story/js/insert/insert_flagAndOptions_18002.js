/******************************************************************************
*@Description : insert, test flag and options                                
*               seqDB-18002:主子表/分区表，插入数据索引键冲突
*@Author      : 2019-3-13  XiaoNi Huang
******************************************************************************/
main();

function main()//TODO :建议将用例里面的tab键改为空格
{  
   if( commIsStandalone( db ) )
   {
   	println(" Deploy mode is standalone!");
		return;
   }
   
	println("\n---Begin to run test");
   var mainCLName = "mcl18002" ;
   var subCLName  = "scl18002" ;
	var cs = db.getCS( COMMCSNAME );
	
	commDropCL( db, COMMCSNAME, mainCLName, true, true, "Failed to drop maincl in the pre-condition." );
	commDropCL( db, COMMCSNAME, subCLName, true, true, "Failed to drop subcl in the pre-condition." );
               
   // ready cl, the subcl is sharding cl
   var cl = cs.createCL( mainCLName, {ShardingKey:{a:1},IsMainCL:true} );  
   cs.createCL( subCLName, {ShardingKey:{a:1}} );
   cl.attachCL( csName +"."+ subCLName, {LowBound:{a:1},UpBound:{a:100}} );
	cl.createIndex( "idx", {a:1}, true, true );//TODO :1.建议这里使用公共方法创建索引commCreateIndex 2.这里使用的是单键索引，与文本用例中描述不符，可修改下文本用例
	
   // test
   cl.insert( {a:1} );//TODO :这里是不是少覆盖了一个测试点，option为SDB_INSERT_RETURN_ID
   // SDB_INSERT_CONTONDUP
	println("\n---Begin to insert, flag[SDB_INSERT_CONTONDUP]");
	var recsArray = [{a:1,b:1},{a:2}];
	var rc = cl.insert( recsArray, SDB_INSERT_CONTONDUP );
	var expRecs = [{"a":1},{"a":2}];
   checkRecords( cl, expRecs );
   
   // SDB_INSERT_REPLACEONDUP
	println("\n---Begin to insert, flag[SDB_INSERT_REPLACEONDUP]");
	var recsArray = [{a:1,b:2},{a:3}];
	var rc = cl.insert( recsArray, SDB_INSERT_REPLACEONDUP );
	var expRecs = [{"a":1,"b":2},{"a":2},{"a":3}];
   checkRecords( cl, expRecs );
   
   cleanCL( mainCLName );
}

function checkRecords( cl, recs ) 
{
   var rc = cl.find( {}, {_id:{$include:0}} ).sort({a:1} );
   var rcRecs = new Array();
   while( tmpRecs = rc.next() )
   {
      rcRecs.push( tmpRecs.toObj() );
   }   
   
   var expRecs = JSON.stringify( recs );
   var actRecs = JSON.stringify( rcRecs );
   if( expRecs !== actRecs )
   {
      throw buildException( "checkResult", null, "", expRecs, "  " + actRecs );
   }
}