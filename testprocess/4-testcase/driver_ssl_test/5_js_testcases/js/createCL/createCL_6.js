//create exist collection case1
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME );

try {
	db.dropCS(CSPREFIX_CS);
	}
	catch(e){
		if (e != -34){
			println("unexpected err happened when clear cs:"+e);
			throw e;
			}
		}
		
try{
	var varCS = db.createCS(CSPREFIX_CS);
	}
	catch(e){
		println("failed to create cs,rc="+e);
		throw e;
		}

try
{
   var varCL = varCS.createCL( CSPREFIX_CL,{Compressed:true} ) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e );
   throw e ;
}

var res = false;
try
{
   varCS.createCL( CSPREFIX_CL ,{Compressed:true}) ;
}
catch ( e )
{
   if (e == -22){
   	res = true;
   	}
}
if(!res){
	throw -1;
	}
	
try{
	db.dropCS( CSPREFIX_CS );
	}
	catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

