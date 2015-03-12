//drop collection
//innomal case2
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

try
{
  db.dropCS(CSPREFIX_CS);
}
catch(e)
{
  if ( e != -34)
    {
        println( "unexpected err happened when clear cs:" + e ) ;
        throw e ;
     }
}

CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 
try{

//var claSize = new RSize( CSPREFIX_CS );

//var varCS = db.createCS(CSPREFIX_CS);

//var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});

}catch( e ){
   throw e ;	
}

try
{
  var varCS = db.createCS(CSPREFIX_CS);
}
catch ( e )
{
  println("failed to create cs,rc="+ e );
  throw e ;
}


 try 
{
  varCS.dropCL("bar.cs");
}
catch( e )
{
  if ( e != SDB_INVALIDARG && e != SDB_DMS_NOTEXIST ){
  	println(" error is not " + SDB_INVALIDARG + " \n or error is not " + SDB_DMS_NOTEXIST );
  	throw e;
  }
     
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
