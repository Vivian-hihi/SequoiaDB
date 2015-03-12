// create cs.
// CSname's large is 127. 
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;


var _CSPREFIX = CSPREFIX_CS ;

for(var i = 0 ; i < (128-_CSPREFIX.length ); ++i ){
   CSPREFIX_CS = CSPREFIX_CS+"a";	
}

var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
var res = false;
try
{
   db.createCS(CSPREFIX_CS);
}
catch( e )
{ 

   res = true;

}
if( !res ){
  throw -1;
}
try
{
   db.dropCS(CSPREFIX_CS);
}
catch( e )
{ 

}

