/*****************************************************************************
@Description : Creating index by use chinese.
@Modify list :
               2014-5-18  xiaojun Hu  Modify
*****************************************************************************/
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
   if ( e != -34 )
   {
      println( "unexpected err happened when clear cs:"+e ) ;
      throw e ;
   }
}

try
{
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.createCS(CSPREFIX_CS);
   var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize(),Compressed:true}) ;
}
catch( e )
{
   println( "Failed to create CS and CL, rc="+e ) ;
   throw e ;
}

try
{
   varCL.createIndex( "chen" , {"中文":1} , true ) ;
}
catch ( e )
{
   println( "Failed to create index, rc="+e ) ;
   throw e;
}

try
{
   varCL.insert({"中文":12}) ;
}
catch( e )
{
   println( "Failed to insert data after create index, rc="+e ) ;
   throw e ;
}

try
{
   varCL.insert({"中文":12}) ;
}
catch( e )
{
   if ( -38 != e )
   {
      println( "Failed to insert same record to database, rc="+e ) ;
      throw e ;
   }
}

try
{
   varCL.createIndex( "testindex", {"use.id":1}, true ) ;
}
catch ( e )
{
   println( "failed to create index, rc="+e ) ;
   throw e ;
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "unexpected err happened when clear cs end:" + e ) ;
   throw e ;
}
