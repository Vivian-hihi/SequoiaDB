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
   varCL.insert({a:1}) ;
   varCL.insert({b:[1,2],salary:10,name:"Tom"}) ;
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
   println( "failed to read record, rc= " + e ) ;
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
println("size=" + size);
if ( 2 != size )
{
   println( " get the nnumber of records is wrong" ) ;
   throw -1 ;
}

try
{
   varCL.remove( {name:"Mike"} ) ;
}
catch ( e )
{
   println( "failed to remove record, rc1= " + e ) ;
   throw e ;
}

try
{
   rc1 = varCL.find({name:"Tom"}) ;
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
   throw e ;
}

size = 0 ;
while ( true )
{
   var i = rc1.next() ;
   if ( !i )
      break ;
   else
      size++ ;
}

if ( 1 != size )
{
   println( " get the number of records is wrong,rc1" ) ;
   throw -1 ;
}

try
{
   varCL.remove( {a:1} ) ;
}
catch ( e )
{
   println( "failed to remove record, rc= " + e ) ;
   throw e ;
}

try
{
   rc = varCL.find({a:1}) ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

size = 0 ;
while ( true )
{
   var i = rc.next() ;
   if ( !i )
      break ;
   else
      size++ ;
}

if (0 != size )
{
   println( " get more than zero record" ) ;
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

