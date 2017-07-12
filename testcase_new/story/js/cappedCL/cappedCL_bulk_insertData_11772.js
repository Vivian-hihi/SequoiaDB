/************************************
*@Description: 批量插入记录,_id超过整型范围
*@author:      luweikang
*@createdate:  2017.7.11
*@testlinkCase:seqDB-11772
**************************************/

main()

function main()
{
   var csName = CHANGEDPREFIX + "_11772_CS";
   var clName = CHANGEDPREFIX + "_11772_CL";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   //create cappedCL
   var optionObj = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:false};
   var dbcl =  commCreateCLByOption( db, csName, clName, optionObj, false, false, "create cappedCL" );
   
   //insertData
   println( "---bulk insert data---" )
   for( var i = 0; i < 13; i++ )
   {
      bulkInsertData( dbcl );
      popData( dbcl );
   }
   
   //check id
   checkId( dbcl );
   
   //clean environment after test  
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}

function bulkInsertData( dbcl )
{
   var doc = new Array();
   var str = createBigStr();
   for( var i = 0; i < 10; i++ )
   {
      var options = { No : i, a : str };
      doc.push( options );
   }
   try
   {
      dbcl.insert( doc );
   }
   catch ( e )
   {
      throw buildException( "bulkInsertData()", e, "bulk insert data", "insert success", "insert fail:"+e );
   }
}

function popData( dbcl )
{
   var rc   = dbcl.find().sort({ "_id":1 }).skip( 9 ).limit( 1 );
   var id  = rc.next().toObj()._id;
   var obj = { LogicalID:id, Direction:1 };
   try
   {
      dbcl.pop( obj );
   }
   catch( e )
   {
      throw buildException( "popData()", e, "pop data", "pop success", "pop fail:"+e );
   }
}

function checkId( dbcl )
{
   try
   {
      dbcl.insert( { a : 1 } );
      var cursor = dbcl.find( null,{ '_id': "" });
      var id = cursor.next().toObj()._id;
      if( id <= 2147483647 )
      {
         throw "ERR_ID_VALUE";
      }
      cursor.close();
   }
   catch ( e )
   {
      throw buildException( "checkId()", e, "check Id", "find success", "find fail:"+e );
   }
}

function createBigStr()
{
   var arr = new Array(16*1024*1023);
   var str = arr.toString();
   return str;
}