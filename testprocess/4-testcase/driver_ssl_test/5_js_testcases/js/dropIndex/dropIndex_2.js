/***************************************************************************
@Description : drop the index ,but index don't exist.
@Modify list :
               2014-5-16  xiaojun Hu  modify
***************************************************************************/
// drop  not exist index.
var CSPREFIX_CS = COMMCSNAME ;
var CSPREFIX_CL = COMMCLNAME ;

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
   var varCS = db.createCS( CSPREFIX_CS ) ;
   var varCL = varCS.createCL( CSPREFIX_CL ,{ReplSize:claSize.ReplSize() , Compressed:true}) ;
}
catch ( e )
{
   println( "failed to create CS and CL rc= " + e );
   throw e ;
}

try
{
   var rc = varCS.getCL( CSPREFIX_CL ) ;
}
catch ( e )
{
   println( "failed to get cl, rc= " + e );
   throw e ;
}

var res = false ;
try
{
   varCL.dropIndex( "testindex" ) ;
}
catch ( e )
{
   if ( e != -47 )
   {
      println( "Failed to drop index :"+e ) ;
      throw e ;
   }
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to clear cs end: " + e ) ;
   throw e ;
}
