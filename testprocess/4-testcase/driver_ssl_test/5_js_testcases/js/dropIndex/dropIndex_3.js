/************************************************************************
@Description : drop the index "$id".Cannot drop .
@Modify list :
               2014-5-16  xiaojun Hu  modify
************************************************************************/
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
   var varCS = db.createCS( CSPREFIX_CS ) ;
   var claSize = new RSize( CSPREFIX_CS ) ;
   var varCL = varCS.createCL( CSPREFIX_CL ,{ReplSize:claSize.ReplSize() , Compressed:true}) ;
}
catch ( e )
{
   println( "Failed to create CS and CL, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.dropIndex( "$id" ) ;
}
catch ( e )
{
   if ( e != -56 )
   {
      println( "Failed to drop index ,rc= "+e ) ;
      throw e ;
   }
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
    println( "failed to clear cs:" + e ) ;
    throw e ;
}
