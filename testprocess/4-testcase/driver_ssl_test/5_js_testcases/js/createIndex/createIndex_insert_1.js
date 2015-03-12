/****************************************************************************
@Description : Creating index and get the index .
@Modify list :
               2014-5-18  xiaojun Hu  Modify
****************************************************************************/
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
   varCL.createIndex( "testindex", {a:1}, true ) ;
}
catch ( e )
{
   println( "failed to create index, rc="+e ) ;
   throw e ;
}

try
{
   varCL.insert({a:1,"name":"hihao"}) ;
}
catch ( e )
{
   println( "failed to insert record, rc="+e ) ;
   throw e ;
}

var index ;
try
{
   index = varCL.getIndex("testindex") ;
}
catch ( e )
{
   println( "failed to get index, rc="+e ) ;
   throw e ;
}

index = index.toString();
index = eval('('+ index +')') ;
var _index = index[ "IndexDef" ] ;
//var _index = eval("("+_index+")") ;
if ( "testindex" != _index["name"] )
{
   println( "wrong index name:" + _index["name"] ) ;
   throw "ErrWrongIndex" ;
}

//_index = eval("("+index["key"]+")") ;
if ( 1 != _index["key"]["a"])
{
   println( "wrong index def:" + index["key"] ) ;
   throw "ErrIdxDef" ;
}
if ( true != _index["unique"] )
{
   println( "wrong index unique" ) ;
   throw "ErrIdxUnique" ;
}

var jx = 0;
try
{
   varCL.insert({a:1,"name":"hihao1232"}) ;
}
catch ( e )
{
   if ( -38 != e )
   {
      println( "Failed to inset data to database after create index, rc="+e ) ;
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

