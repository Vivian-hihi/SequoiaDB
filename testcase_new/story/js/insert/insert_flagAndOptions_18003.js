/******************************************************************************
*@Description : insert, test flag and options                                
*               seqDB-18003:批量插入，多个options组合使用，插入数据冲突
*@Author      : 2019-3-13  XiaoNi Huang
******************************************************************************/
main();

function main()
{  
	println("\n---Begin to run test");
	var clName = COMMCLNAME + "_18003";
	var idxName = "idx";	
   var cl = readyCL( clName );
	cl.createIndex( idxName, {a:1, b:1}, true, true );
   
   // test
   setOptions_RC_true( cl );
   setOptions_RR_true( cl );
   setOptions_RC_false( cl );
   setOptions_RR_false( cl );
   setOptions_CR( cl );
   
   cleanCL( clName );
}

function setOptions_RC_true( cl )
{
	println("\n---Begin to insert, set options[{ReturnOID:true,ContOnDup:true}].");
   cl.insert( {a:1,b:1} );
   
	var recsArray = [{a:1,b:1,c:1},{a:2}];
	var rc = cl.insert( recsArray, {ReturnOID:true,ContOnDup:true} );
   checkReturnOid( rc );
	var expRecs = [{"a":1,"b":1},{"a":2}];
   checkRecords( cl, expRecs );
   cl.remove();
}

function setOptions_RR_true( cl )
{
	println("\n---Begin to insert, set options[{ReturnOID:true,ReplaceOnDup:true}].");
   cl.insert( {a:1,b:1} );
   
	var recsArray = [{a:1,b:1,c:2},{a:3}];
	var rc = cl.insert( recsArray, {ReturnOID:true,ReplaceOnDup:true} );
   checkReturnOid( rc );
	var expRecs = [{"a":1,"b":1,"c":2},{"a":3}];
   checkRecords( cl, expRecs );
   cl.remove();
}

function setOptions_RC_false( cl )
{
	println("\n---Begin to insert, set options[{ReturnOID:false,ContOnDup:false}].");
   cl.insert( {a:1,b:1} );
	
	var recsArray = [{a:1,b:1,c:3},{a:3}];
   try
   {
	   cl.insert( recsArray, {ReturnOID:false,ContOnDup:false} );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -38 !== e )
      {
   	   throw e;
   	}
   }
   
	var expRecs = [{"a":1,"b":1}];
   checkRecords( cl, expRecs );
   
   cl.remove();
}

function setOptions_RR_false( cl )
{
	println("\n---Begin to insert, set options[{ReturnOID:false,ReplaceOnDup:false}].");
   cl.insert( {a:1,b:1} );
   
	var recsArray = [{a:1,b:1,c:4},{a:4}];
   try
   {
	   cl.insert( recsArray, {ReturnOID:false,ReplaceOnDup:false} );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -38 !== e )
      {
   	   throw e;
   	}
   }
   
	var expRecs = [{"a":1,"b":1}];
   checkRecords( cl, expRecs );
   
   cl.remove();
}

function setOptions_CR( cl )
{
	println("\n---Begin to insert, set options[{ContOnDup:true,ReplaceOnDup:true}].");
   cl.insert( {a:1,b:1} );
   
	// ContOnDup:true,ReplaceOnDup:true
	var recsArray = [{a:1,b:1,c:5},{a:5}];
   try
   {
	   cl.insert( recsArray, {ContOnDup:true,ReplaceOnDup:true} );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -6 !== e )
      {
   	   throw buildException( "setOptions_CR", null, "", -6, "  " + -38 );
   	}
   }   
	var expRecs = [{"a":1,"b":1}];
   checkRecords( cl, expRecs );
   
   // ContOnDup:true,ReplaceOnDup:false
	var recsArray = [{a:1,b:1,c:6},{a:6}];
   cl.insert( recsArray, {ContOnDup:true,ReplaceOnDup:false} );
	var expRecs = [{"a":1,"b":1},{"a":6}];
   checkRecords( cl, expRecs );
   
   // ContOnDup:false,ReplaceOnDup:true
	var recsArray = [{a:1,b:1,c:7},{a:7}];
   cl.insert( recsArray, {ContOnDup:false,ReplaceOnDup:true} );
	var expRecs = [{"a":1,"b":1,"c":7},{"a":6},{"a":7}];
   checkRecords( cl, expRecs );
   
   // ContOnDup:false,ReplaceOnDup:false
	var recsArray = [{a:1,b:1,c:8},{a:8}];
   try
   {
	   cl.insert( recsArray, {ContOnDup:false,ReplaceOnDup:false} );
      throw "expect fail, but actual succ." 
   }
   catch(e)
   {
      if( -38 !== e )
      {
   	   throw e;
   	}
   }
   
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
   for( var i = 0; i < rc.length; i++ )
   {
      var oid = rc[i].toObj()["_id"]["$oid"];
      var expTypeOid = "string";
      var actTypeOid = typeof( oid );
      if( expTypeOid !== actTypeOid )
      {
         throw buildException( "checkReturnOid", null, "", expTypeOid, "  " + actTypeOid );
      } 
   }
}