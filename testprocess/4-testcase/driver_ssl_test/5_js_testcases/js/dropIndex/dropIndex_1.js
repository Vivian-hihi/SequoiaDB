/*********************************************************************************
@Description : create the index first, and then drop the index normal.
@Modify list :
               2014-5-16  xiaojun Hu Modify
*********************************************************************************/
var CSPREFIX_CS = COMMCSNAME ;
var CSPREFIX_CL = COMMCLNAME ;

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( -34 != e )
   {
      println( "Failed to clear the collectionspace first :"+e ) ;
      throw e ;
   }
}

try
{
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.createCS(CSPREFIX_CS);
   var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize(),Compressed:true});
}
catch( e )
{
   println( "Failed to create CS and CL :"+e ) ;
   throw e ;
}

//create index
try
{
   varCL.createIndex( "testindex", {a:1}, false ) ;
   inspecIndex( varCL, "testindex" , "a" , 1 , false , false ) ;
}
catch ( e )
{
   println( "failed to create index, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.dropIndex( "testindex" ) ;
}
catch ( e )
{
   println( "failed to drop index, rc= " + e ) ;
   throw e ;
}

commCheckIndex( varCL, "testindex", false ) ;

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "Failed to drop CS in the end :"+e ) ;
   throw e ;
}

