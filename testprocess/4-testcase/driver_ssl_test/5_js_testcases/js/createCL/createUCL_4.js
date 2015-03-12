// create cl.
// normal case.

CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;  
var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME ) ;

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch (e)
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
}
catch ( e )
{
   println( "failed to create cs, rc= " + e );
   throw e ;
}

var j = 0;
var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*","(",")");
for(var i = 0 ; i < aa.length ; ++i ){
   	try{
   	    var CLname = CSPREFIX + aa[i];	
   	    var varCL = varCS.createCL(CLname);
   	}catch( e ){
   			++j ;
   	}
}

if( 0 != j ){
   throw -1 ; 
}


try
{
   var rc = varCS.getCL( CSPREFIX+ aa[3] ) ;
}
catch ( e )
{
   println( "failed to get cl, rc= " + e );
   throw e ;
}

try
{
   varCL.insert({a:1}) ;
}
catch ( e )
{
   println( "failed to insert record to cl, rc= " + e );
   throw e ;
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

