// insert record.
// normal case.
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
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

try{

var claSize = new RSize( CSPREFIX_CS );

var varCS = db.createCS(CSPREFIX_CS);

var len = claSize.ReplSize();

if( 1 == len && -1 != len ){
   var len = 1 ;
}else{
   var len = len - 1 ;
}

var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:len},{Compressed:true});

}catch( e ){
   throw e ;	
}
try
{
   varCL.insert({a:1}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

var rc ;

var j = 0 ; 

sleep(3000);

try
{
	rc = varCL.find({a:1}) ;
}
catch ( e )
{
	println( "failed to read record, rc= " + e ) ;
	throw e;
}

var size = 0 ;
while ( true )
{
  var i = rc.next() ;
  if ( !i )
     break ;
	else
	   size++ ;
}
	
if ( 1 != size )
{
   println( " get more than one record" ) ;
   throw -1 ;
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
