/********************************************************************
@Description : create unique index.
@Modify list :
               2014-5-18  xiaojun Hu  Modify
********************************************************************/
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
   var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize(),Compressed:true});
}
catch( e )
{
   println( "Failed to create CS and CL, rc="+e ) ;
   throw e ;
}


try
{
   varCL.createIndex( "testindex", {"name":1}, true ) ;
}
catch ( e )
{
   println( "failed to create index, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.insert({a:1,"name":"hihao"}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.insert({a:1,"name":"xixie"}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;

}

try
{
   varCL.insert({a:1,"name":"hihao"}) ;
}
catch ( e )
{
   if ( -38 != e )
   {
      println( "Failed to insert data to database after create index, rc="+e ) ;
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

