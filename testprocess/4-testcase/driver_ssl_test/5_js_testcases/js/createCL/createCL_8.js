//create exist collection case3
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CS1 = CSPREFIX+"foo1" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME )

try {
	db.dropCS(CSPREFIX_CS);
	}
	catch(e){
		if (e != -34){
			println("unexpected err happened when clear cs:"+e);
			throw e;
			}
		}

try {
	db.dropCS(CSPREFIX_CS1);
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

try{
	var varCS1 = db.createCS(CSPREFIX_CS1);
	}
	catch(e){
		println("failed to create cs,rc="+e);
		throw e;
		}
		
try
{
   varCS.createCL( CSPREFIX_CL,{Compressed:true}) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e );
   throw e ;
}

try
{
   varCS1.createCL( CSPREFIX_CL ,{Compressed:true}) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e );
   throw e ;
}

try{
	db.dropCS( CSPREFIX_CS );
	}
	catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

try{
	db.dropCS( CSPREFIX_CS1 );
	}
	catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}
