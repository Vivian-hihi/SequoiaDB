//create a index which tne key is more than 1000B
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
hostName = COORDHOSTNAME ;
coordPort = COORDSVCNAME ;

var db = new SecureSdb( hostName, coordPort ) ;
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


try
{
   varCL.createIndex( "testindex", {a:1},false) ;
}
catch ( e )
{
   println("failed to create index");
   throw e ;
}


try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
  println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

