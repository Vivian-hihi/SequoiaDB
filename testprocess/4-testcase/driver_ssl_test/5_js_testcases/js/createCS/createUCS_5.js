// create cs.
// CSname's large is 127. 
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
for(var i = 0 ; i < (127-CSPREFIX_CS.length) ; ++i ){
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
    throw e;
}
try
{
   db.dropCS(CSPREFIX_CS);
}
catch( e )
{ 
	println("failed to clear the 127B CS , rc ="+e);
	throw e ;
}



