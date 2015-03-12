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
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
try{

var claSize = new RSize( CSPREFIX_CS );

var varCS = db.createCS(CSPREFIX_CS);

var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()},{Compressed:true});

}catch( e ){
   throw e ;
}

var nestedLevel=33;
var insert_string = "" ;
function str_name(jx)
{
   if( jx < nestedLevel ){
      jx++;
      return ( "{ abcdefjhijklmopqrst: "+str_name(jx)+"}" )
   }else{
     return "{abcdefjhijklmopqrst:100}";
   }

}

insert_string = eval("("+str_name(1)+")");

var bCatchException = false;
try
{
   varCL.insert(insert_string) ;

}
catch ( e )
{
   throw "insert the big bson fail";
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
