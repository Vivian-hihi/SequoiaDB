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

var str_1 = "\"" ; 
for(var i = 0 ; i < 1000 ; ++i ){
   
   str_1 = str_1+"che" ; 
   	
}
str_1 = str_1+"\"";

var str_2 = "{name:\"qiu\",balance:1.2}" ; 
for(var i = 0 ; i < 1000 ; ++i ){
   
   str_2 = str_2+",{name:\"qiu\",balance:1.2}" ; 
   	
}





var insert_string = "{str:"+str_1+" , array:["+str_2+"]}" ; 

 insert_string = eval("("+insert_string+")");
try
{
   varCL.insert(insert_string) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find(insert_string) ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}
rc = rc.toArray();
if( 1 != rc.length ){
   throw -1 ; 	
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
