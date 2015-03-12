/*******************************************************************************
@Description : Create Index common functions
@Modify list :
               2014-5-20  xiaojun Hu  Init
*******************************************************************************/
var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;
var csName = COMMCSNAME ;
var clName = COMMCLNAME ;

var db = new SecureSdb( hostName, coordPort ) ;

// common functions
function createIndex( cl, idxName, idxKeygen, unique, enforced , errno )
{
   if ( undefined == unique ) { unique = false ; }
   if ( undefined == enforced ) { enforced = false ; }
   if ( undefined == errno ) { errno = "" ; }
   try
   {
      if( undefined == cl || undefined == idxName || undefined == idxKeygen )
      {
         println( "please check the argument of createIndex" ) ;
         throw "ErrArg" ;
      }
      cl.createIndex( idxName, idxKeygen, unique, enforced ) ;
      // inspect the index we created
   }
   catch ( e )
   {
      if ( errno != e )
      {
         println( "failed to create index, rc = "+ e ) ;
         throw e ;
      }
   }
}

//inspect the index is created success or not.
function inspecIndex( cl, indexName , indexKey , keyValue , idxUnique , idxEnforced )
{
   if ( undefined == idxUnique ) { idxUnique = false ; }
   if ( undefined == idxEnforced ) { idxEnforced = false ; }
   try
   {
      if ( undefined == cl || undefined == indexName || undefined == indexKey || undefined == keyValue )
      {
         println( " wrong argument when inspect index " ) ;
         throw "ErrArg" ;
      }
      var getIndex = new Boolean(true) ;
      getIndex = cl.getIndex( indexName ) ;
      var cnt = 0 ;
      while ( cnt < 20 )
      {
         getIndex = cl.getIndex( indexName ) ;
         if ( undefined != getIndex )
         {
            break ;
         }
         ++cnt ;
      }
      if ( undefined == getIndex )
      {
         println( "Don't have the index, name = "+indexName ) ;
         throw "ErrIdxName" ;
      }
      //println(cl.getIndex( indexName )) ;
      var indexDef = getIndex.toString() ;
      indexDef = eval('('+indexDef+')') ;
      var index = indexDef[ "IndexDef" ] ;
      if ( keyValue != index["key"][indexKey] )
      {
         println( "Wrong index name or key value : "+index["key"][indexKey] ) ;
         throw "ErrIdxValue" ;
      }
      if ( idxUnique != index["unique"] )
      {
         println( "Wrong index unique : "+index["unique"] ) ;
         throw "ErrIdxUnique" ;
      }
      if ( idxEnforced != index["enforced"] )
      {
         println( "Wrong index enforced : "+index["enforced"] ) ;
         throw "ErrIdxEnforced" ;
      }
      println("Success to inspect index : "+indexName) ;
   }
   catch ( e )
   {
      println( "argument value:'"+indexName+"','"+indexKey+"','"+keyValue+"','"+idxUnique+"','"+idxEnforced ) ;
      println( "Failed to inspect index : "+indexName+" rc=: "+e ) ;
      throw e ;
   }
}
