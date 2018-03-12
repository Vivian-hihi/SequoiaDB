/*******************************************************************
* @Description : test case for SdbQuery and SdbCursor
*                seqDB-14664:使用arrayAccess获取SdbCursor中指定下标元素
*                seqDB-14665:使用下标获取SdbQuery中指定元素
*                seqDB-14666:指定flags执行SdbQuery
*                seqDB-14667:使用close关闭SdbQuery
*                seqDB-14668:执行getQueryMeta获取SdbQuery的查询元数据              
* @author      : Liang XueWang
*                2018-03-12
*******************************************************************/
var clName = COMMCLNAME + "_dbClasses14664" ;

main( db ) ;

function main( db )
{
   var cl = commCreateCL( db, COMMCSNAME, clName, 0 ) ;
   
   cl.insert( { a: 1 } ) ;
   cl.insert( { a: 2 } ) ;
   
   cursorArrayAccess( cl ) ;
   queryIndex( cl ) ;
   queryArrayAccess( cl ) ;
   queryFlags( cl ) ;
   queryClose( cl ) ;
   queryMeta( cl ) ;
   
   commDropCL( db, COMMCSNAME, clName ) ;
}

function cursorArrayAccess( cl )
{
   var cursor = cl.find().sort( { a: 1 } ) ;
   
   var a1 = JSON.parse( cursor.arrayAccess(0) )["a"] ;
   var a2 = JSON.parse( cursor.arrayAccess(1) )["a"] ;
   if( a1 !== 1 || a2 !== 2 )
   {
      throw buildException( "cursorArrayAccess", null, "check arrayAccess",
            "1 2", a1 + " " + a2 ) ;
   }
   
   var illegalIdx = [ 'b', -1, 1.2, 2 ] ;
   for( var i = 0;i < illegalIdx.length;i++ )
   { 
      if( cursor.arrayAccess( illegalIdx[i] ) !== undefined )
      {
         println( "check arrayAccess with idx " + illegalIdx[i] ) ;
         throw buildException( "cursorArrayAccess", null ) ;
      }
   }
}

function queryIndex( cl )
{
   var a1 = JSON.parse( cl.find().sort( { a: 1 } )[0] )["a"] ;
   var a2 = JSON.parse( cl.find().sort( { a: 1 } )[1] )["a"] ;
   if( a1 !== 1 || a2 !== 2 )
   {
      throw buildException( "queryIndex", null, "check queryIndex",
            "1 2", a1 + " " + a2 ) ;
   }
   
   var illegalIdx = [ 'b', -1, 1.2, 2 ] ;
   for( var i = 0;i < illegalIdx.length;i++ )
   { 
      if(  cl.find().sort( { a: 1 } )[ illegalIdx[i] ] !== undefined )
      {
         println( "check queryIndex with idx " + illegalIdx[i] ) ;
         throw buildException( "queryIndex", null ) ;
      }
   }
}

function queryArrayAccess( cl )
{
   var a1 = JSON.parse( cl.find().sort( { a: 1 } ).arrayAccess( 0 ) )["a"] ;
   var a2 = JSON.parse( cl.find().sort( { a: 1 } ).arrayAccess( 1 ) )["a"] ;
   if( a1 !== 1 || a2 !== 2 )
   {
      throw buildException( "queryArrayAccess", null, "check queryArrayAccess",
            "1 2", a1 + " " + a2 ) ;
   }
   
   var illegalIdx = [ 'b', -1, 1.2, 2 ] ;
   for( var i = 0;i < illegalIdx.length;i++ )
   { 
      if(  cl.find().sort( { a: 1 } ).arrayAccess( illegalIdx[i] ) !== undefined )
      {
         println( "check queryArrayAccess with idx " + illegalIdx[i] ) ;
         throw buildException( "queryArrayAccess", null ) ;
      }
   }
}

function queryFlags( cl )
{
   try
   {
      cl.find().flags( 0 ) ;
   }
   catch( e )
   {
      throw buildException( "queryFlags", e, "query with flags 0", 0, e ) ;
   }
   try
   {
      cl.find().flags( 'a' ) ;  // nothrow
   }
   catch( e )
   {
      println( e ) ;
   }
}
   
function queryClose( cl )
{
   try
   {
      cl.find().close() ;
   }
   catch( e )
   {
      throw buildException( "queryClose", e, "close query", 0, e ) ;
   }
}

function queryMeta( cl )
{
   var scanType = cl.find().getQueryMeta().next().toObj()["ScanType"] ;
   if( scanType !== "tbscan" )
   {
      throw buildException( "queryMeta", null, "check getQueryMeta", 
            "tbscan", scanType ) ;
   }
} 