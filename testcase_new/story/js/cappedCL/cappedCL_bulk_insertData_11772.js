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
   var optionObj = {Capped:true, Size:4294967296, Max:10000000, AutoIndexId:false};
   var dbcl =  commCreateCLByOption( db, csName, clName, optionObj, false, false, "create cappedCL" );
   
   //insertData
   println( "---bulk insert data---" )
   for( var i = 0; i < 20; i++ )
   {
      bulkInsertData( dbcl );
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
   for( var i = 0; i < 100; i++ )
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

function checkId( dbcl )
{
   try
   {
      //dbcl.insert( { a : 1 } );
      var cursor = dbcl.find( null,{ '_id': "" }).sort({ "_id":1 }).skip( 1999 ).limit( 1 );
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
   var arr = new Array( 1024 * 1024 );
   var str = arr.toString();
   return str;
}