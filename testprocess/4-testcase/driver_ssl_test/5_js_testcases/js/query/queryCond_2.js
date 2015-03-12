// query record.
// normal matcher $and $or $not

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
   throw e ;	
}

var rc = "" ;
try
{
   rc = varCL.find({$and:[{name:"chen"},{age:12}]}) ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}
if( "" != rc ){
   throw -1;	
}

var rc = "" ;
try
{
   rc = varCL.find({$or:[{name:"chen"},{age:115}]}) ;
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
   throw e ;
}

if( "" != rc ){
   throw -1;	
	
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
