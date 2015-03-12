// query record.
// normal matcher
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
   varCL.insert({a:1}) ;
   varCL.insert({b:11}) ;
   varCL.insert({c:13}) ;
   varCL.insert({d:15}) ;
   varCL.insert({e:17}) ;
   varCL.insert({f:19}) ;
   
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find() ;
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
   throw e ;
}

try
{
   varCL.find({a:1}) ;
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
   throw e ;
}
try
{
   varCL.find({b:{$gt:10}}) ;
}
catch ( e )
{
   println( "failed to read record, rc2= " + e ) ;
   throw e ;
}
try
{
   varCL.find({c:{$gte:13}}) ;
}
catch ( e )
{
   println( "failed to read record, rc3= " + e ) ;
   throw e ;
}
try
{
   varCL.find({d:{$lt:16}}) ;
}
catch ( e )
{
   println( "failed to read record, rc4= " + e ) ;
   throw e ;
}
try
{
   varCL.find({e:{$lte:17}}) ;
}
catch ( e )
{
   println( "failed to read record, rc5= " + e ) ;
   throw e ;
}
try
{
   varCL.find({f:{$ne:12}}) ;
}
catch ( e )
{
   println( "failed to read record, rc6= " + e ) ;
   throw e ;
}

var size = 0 ;
while ( true )
{
   var i = rc.next() ;
   if ( !i )
      break ;
   else
      size++ ;
}

if ( 6 != size )
{
   println( " get more than one record" ) ;
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
