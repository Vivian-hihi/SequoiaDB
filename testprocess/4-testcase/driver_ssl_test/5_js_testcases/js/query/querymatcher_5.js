// query record.
// normal matcher $regex $options
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
	 varCL.insert({name:"Tom"});
	 varCL.insert({name:"tom"});
	 varCL.insert({name:"Mike"});
	 varCL.insert({name:"Davidely"});
	 varCL.insert({name:"Ddrtytavideely"});
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find({name:{$regex:"^t",$options:"i"}}) ;
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

if ( 2 != size )
{
   println( " get the number of records is wrong.." ) ;
   throw -1 ;
}

var rc1 ;
try
{
   rc1 = varCL.find({name:{$regex:"e{2}",$options:"i"}}) ;
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
   throw e ;
}

var size1 = 0 ;
while ( true )
{
   var j = rc1.next() ;
   if ( !j )
      break ;
   else
      size1++ ;
}

if ( 1 != size1 )
{
   println( " get the number of records is wrong...." ) ;
   throw -1 ;
}

var rc2 ;
try
{
   rc2 = varCL.find({name:{$regex:"e$",$options:"i"}}) ;
}
catch ( e )
{
   println( "failed to read record, rc2= " + e ) ;
   throw e ;
}

var size2 = 0 ;
while ( true )
{
   var j = rc2.next() ;
   if ( !j )
      break ;
   else
      size2++ ;
}

if ( 1 != size2 )
{
   println( " get the number of records is wrong......" ) ;
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
