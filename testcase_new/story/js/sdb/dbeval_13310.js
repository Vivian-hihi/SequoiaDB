/******************************************************************************
*@Description : test get collection with eval
*               TestLink : seqDB-13310:执行eval获取集合后插入查询数据
*@auhor       : Liang XueWang
******************************************************************************/
function main( db )
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }
   
   var clName = CHANGEDPREFIX + "_cl13310" ;
   commCreateCL( db, COMMCSNAME, clName ) ;
    
   try
   {
      var code = "db." + COMMCSNAME + "." + clName ;
      var cl = db.eval( code ) ;
      cl.insert( { a: 1 } ) ;
   }
   catch( e )
   {
      throw buildException( "main", e, "eval cl " + code, 0, e ) ;
   }
   
   var aVal = cl.find( {}, { a: "" } ).next().toObj()["a"] ;
   if( aVal !== 1 )
   {
      throw buildException( "main", null, "check find result", 1, aVal ) ;
   } 
    
   commDropCL( db, COMMCSNAME, clName ) ;
}

main( db ) ;