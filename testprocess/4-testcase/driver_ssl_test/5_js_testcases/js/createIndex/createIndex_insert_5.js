/*************************************************************************
@Description : Creating index by use nest object as the key .
@Modify list :
               2014-5-18  xiaojun Hu  Modify
*************************************************************************/
var CSPREFIX_CS = CSPREFIX+"foo" ;
var CSPREFIX_CL = CSPREFIX+"bar" ;
var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;

var db = new SecureSdb( hostName , coordPort ) ;
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try
{
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.createCS(CSPREFIX_CS);
   var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()},{Compressed:true});
}
catch( e )
{
   println( "Failed to create CS and CL, rc="+e ) ;
   throw e ;
}

try
{
   varCL.createIndex( "testindex", {"use.id":1}, true ) ;
   inspecIndex( "testindex" , "use.id" , 1 ,true , false ) ;
}
catch ( e )
{
   println( "failed to create index, rc="+e ) ;
   throw e ;
}

try
{
   varCL.insert({use:{id:1,name:"chen"}}) ;
   varCL.insert({use:{id:2,name:"chen"}}) ;
}
catch ( e )
{
   println( "failed to insert record, rc="+e ) ;
   throw e ;
}

try
{
   varCL.insert({use:{id:1,name:"chensdf"}}) ;
}
catch ( e )
{
   if ( -38 != e )
   {
      println( "Failed to insert data to database, rc="+e ) ;
      throw e ;
   }
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
  println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

//inspect the index is created success or not.
function inspecIndex( indexName , indexKey , keyValue , idxUnique , idxEnforced )
{
   try
   {
      var getIndex = new Boolean(true) ;
      getIndex = varCL.getIndex( indexName ) ;
      while ( undefined == getIndex )
      {
         getIndex = varCL.getIndex( indexName ) ;
      }
      //println(varCL.getIndex( indexName )) ;
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

