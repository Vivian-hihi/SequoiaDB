var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME) ;

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ; 

try
{
	db.dropCS(CSPREFIX_CS) ; 
}
catch ( e )
{
	if( -34!=e )
	{
		println("Failed to drop CS ,begin "+e) ;
		throw e ; 
	}
}	

var cl ;	
try
{
	var claSize = new RSize(CSPREFIX_CS) ;	
	var cs = db.createCS(CSPREFIX_CS) ;
	cl = cs.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()}) ;
}
catch ( e )
{
	println("Failed to create CS and CL "+e) ;
	throw e ; 
}

try
{
	cl.insert({a:1}) ; 
	cl.insert({"$abc":"china"}) ;
}
catch ( e )
{
	println("Failed to insert record to CL "+e) ;
	throw e ;
}

//the symbol "$" exit in record,remove?insert?
try
{
	cl.remove({a:1}) ;
//	cl.remove({"$abc":"china"}) ; 
	cl.remove() ;
}
catch ( e )
{
	println("Failed to remove the record "+e) ;
	throw e ;
}

try
{
	var count = cl.count() ;
	if( count!=0 )
	{
		println("Failed to remove records "+count) ;
		throw "Error_Remove" ;
	}
}
catch ( e )
{
	println("Failed remove "+e ) ;
	throw e ;
}

try
{
	db.dropCS(CSPREFIX_CS) ;
}
catch ( e )
{
	println("Failed to drop CS ,end "+e) ;
	throw e ;
}



