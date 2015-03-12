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

var jx = 0 ;
var insert_string = "" ;
function str_name()
{
   if( jx != 999 ){
      jx++;
      return ( "{ a: "+str_name()+"}" )
   }else{
     return "{a:100}";
   }

}

insert_string = eval("("+str_name()+")");

var expectedResult = false ; 
try
{
   varCL.insert(insert_string) ;
}
catch ( e )
{
  throw "insert a big bson fail"; 
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
