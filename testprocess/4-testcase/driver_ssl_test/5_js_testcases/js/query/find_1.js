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

try{

var claSize = new RSize( CSPREFIX_CS );

var varCS = db.createCS(CSPREFIX_CS);

var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()},{Compressed:true});

}catch( e ){
   println("createCS or createCL fail");
   throw e ;	
}

try{
   for( var i=0; i < 100; i++){
     varCL.insert({a:i});
   }
   if( 10 != varCL.find().limit(10).size() )
     throw -1 ;
   if( 100 != varCL.find().limit(10).count() )
     throw -1 ;
   if( 10 != varCL.find().skip(90).size() )
     throw -1;
   if( 100 != varCL.find().skip(90).count() )
     throw -1 ;
}catch(e){
   if( e == -1 ){
      println("find() with limit or skip count/size have error \n");
      throw -1;	
   }
   else{
   	throw e ;
   }

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
