/*******************************************************************
* @Description : test case for CLCount
*                seqDB-14670:使用valueOf获取CLCount值
*                seqDB-14671:指定hint执行CLCount        
* @author      : Liang XueWang
*                2018-03-12
*******************************************************************/
var clName = COMMCLNAME + "_dbClasses14670" ;

main( db ) ;

function main( db )
{
   var cl = commCreateCL( db, COMMCSNAME, clName, 0 ) ;
   
   cl.insert( { a: 1 } ) ;
   cl.insert( { a: 2 } ) ;
   
   var cnt = cl.count().valueOf() ;
   if( cnt !== 2 )
   {
      throw buildException( "main", null, "check valueOf", 2, cnt ) ;
   }
   
   cnt = cl.count().hint( { a: "" } ) ;
   if( parseInt( cnt ) !== 2 )
   {
      throw buildException( "main", null, "check hint", 2, cnt ) ;
   }
   
   try
   {
      cl.count().hint( 1 ) ;  // nothrow
   }
   catch( e )
   {
      println( e ) ;
   }
   
   commDropCL( db, COMMCSNAME, clName ) ;
}