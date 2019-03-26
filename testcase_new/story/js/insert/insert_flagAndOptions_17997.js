/******************************************************************************
*@Description : insert, test flags and options                                 
*               seqDB-17997:insert，原有基本功能验证 
*@Author      : 2019-3-13  XiaoNi Huang
******************************************************************************/
main();

function main()
{  
	println("\n---Begin to run test");//TODO :建议将用例里面的tab键改为空格，用例里面的格式也可以再对齐下
	var clName = "insertFlag_17997";
	var idxName = "idx";	
   var cl = readyCL( clName );
	cl.createIndex( idxName, {a:1, b:1}, true, true );//TODO :建议这里使用公共方法创建索引commCreateIndex
   
   // test
   insertNotSetFlag( cl );
   insertSetFlag_ReturnOid( cl );
   insertSetFlag_ContOnDup( cl );
   
   cleanCL( clName );
}

function insertNotSetFlag( cl )
{
	println("\n---Begin to insert docs, not set flag");
	// index key not conflict
	var recs = [{"a":1},{"a":2}];
   cl.insert( recs );
   
   // index key conflict
   try
   {
      cl.insert( {a:1,c:1} );
      throw "expect fail, but actual succ."  //TODO :建议使用buildException抛出异常，定位问题时可以更清晰，如果觉得太繁琐也可不用
   }
   catch(e)
   {
      if( -38 !== e )
      {
   	   throw e;
   	}
   }
   
   checkRecords( cl, recs );
   
	cl.remove();
}

function insertSetFlag_ReturnOid( cl )
{
	println("\n---Begin to insert docs, set flag[SDB_INSERT_RETURN_ID]/options[ReturnOID]");
	cl.insert({a:1,b:1});
	
	// index key not conflict
   var rc = cl.insert( {a:1,b:2}, SDB_INSERT_RETURN_ID );
   checkReturnOid( rc );
   
   var rc = cl.insert( {a:1,b:3}, {ReturnOID:true} );
   checkReturnOid( rc );
   
   var rc = cl.insert( {a:1,b:4}, {ReturnOID:false} );
   if( null != rc )
   {
      throw buildException( "checkReturnOid", null, "", "not return oid", "  " + "return oid" );
   } 
	
	// index key conflict
   try
   {
      var rc = cl.insert( {a:1,b:1,c:1}, SDB_INSERT_RETURN_ID );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -38 != e )
      {
   	   throw e;
   	}
   }
   
   var expRecs = [{"a":1,"b":1},{"a":1,"b":2},{"a":1,"b":3},{"a":1,"b":4}];
   checkRecords( cl, expRecs );
   
	cl.remove();
}

function insertSetFlag_ContOnDup( cl )
{
	println("\n---Begin to insert docs, set flag[SDB_INSERT_CONTONDUP]/options[ContOnDup]");
	// index key not conflict
	cl.insert([{a:1,b:1}]);
   
   // index key conflict
   // SDB_INSERT_CONTONDUP
   cl.insert( [{a:1,b:1,c:1},{a:2}], SDB_INSERT_CONTONDUP );
   // ContOnDup:true
   cl.insert( [{a:1,b:1,c:2},{a:3}], {ContOnDup:true} );
   
   // ContOnDup:false
   try
   {
      cl.insert( [{a:1,b:1,c:3},{a:4}], {ContOnDup:false} );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -38 !== e )
      {
   	   throw e;
   	}
   }
   
   // insert one doc, flag: SDB_INSERT_CONTONDUP
   try
   {
      cl.insert( {a:5}, SDB_INSERT_CONTONDUP );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -6 !== e )
      {
   	   throw e;
   	}
   }
   
   // insert one doc, options：ContOnDup
   try
   {
      cl.insert( {a:6}, {ContOnDup:true} );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -6 !== e )
      {
   	   throw e;
   	}
   }
   
   var expRecs = [{"a":1,"b":1},{"a":2},{"a":3}];
   checkRecords( cl, expRecs );
   
	cl.remove();
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

function checkReturnOid( rc ) {
   var oid = rc.toObj()["_id"]["$oid"];
   var expTypeOid = "string";
   var actTypeOid = typeof( oid );//TODO :这里比较的是Oid的类型是否为string，但是没有比较Oid的值是否正确
   if( expTypeOid !== actTypeOid )
   {
      throw buildException( "checkReturnOid", null, "", expTypeOid, "  " + actTypeOid );
   } 
}