// delete record.
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

try
{
   varCL.remove() ;
}
catch ( e )
{
   println( "failed to remove record, rc1= " + e ) ;
   throw e ;
}

var size = -1;
try
{
   size = varCL.count() ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

println("size=" + size);
if ( 0 != size )
{
   println( "The records count is not equal 0 after remove all." ) ;
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

