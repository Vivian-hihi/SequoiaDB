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
  varCL.insert({a:4});
  varCL.insert({a:5});
  for(var i=0;i<100;i++){
     varCL.insert({b:i});	
  }	
  
  if( ( 1 != varCL.find({a:{$mod:[4,0]}}).count() ) && ( 4 != varCL.find({a:{$mod:[4,0]}}).current().toObj()["a"] ) )
     throw -1;
  if( ( 1 != varCL.find({a:{$mod:[4,1]}}).count() ) && ( 5 != varCL.find({a:{$mod:[4,0]}}).current().toObj()["a"] ) )
     throw -1;
	if( 2 != varCL.find({b:{$mod:[99,0]}}).count() )
	   throw -1;
	if( 100 != varCL.find({b:{$mod:[1,0]}}).count() ) 
	   throw -1;
}catch( e ){
  if( e == -1 ){
     println("find with $mod have error \n")
     throw -1;	
  }else
  	 throw e;
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