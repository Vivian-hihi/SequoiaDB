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
   var curr = varCL.find().skip(97).sort({a:1});
   if( 97 != curr.current().toObj()["a"] )
     throw -1;
   
   var curr = varCL.find().skip(97).sort({a:-1});
   if( 2 != curr.current().toObj()["a"] ) 
     throw -1 ;
   
   var curr = varCL.find().skip(97).limit(2).sort({a:-1});
   if( 100 != curr.count() )
     throw -2;  
   if( 2 != curr.size() )
     throw -2;
   var curr = varCL.find().skip(97).limit(2).sort({a:-1});
   while( curr.next() ){ var curr_before = curr.current(); }
   if( 1 != curr_before.toObj()["a"] )
     throw -2;
     
   var curr = varCL.find().skip(97).limit(2).sort({a:1});
   if( 100 != curr.count() )
     throw -2;  
   if( 2 != curr.size() )
     throw -2;
   var curr = varCL.find().skip(97).limit(2).sort({a:1});
   while( curr.next() ){ var curr_before = curr.current(); }
   if( 98 != curr_before.toObj()["a"] ) 
     throw -2;
     
     
}catch(e){
   if( e == -1 ){
      println("find() with (skip~sort[1/-1]) have error \n");
      throw -1;	
   }
   else if( e == -2){
      println("find() with (skip~limit~sort[1/-1]) have error \n") ;
      throw -2;	
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