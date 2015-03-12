// query record.
// normal matcher $in $nin $all

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
	 varCL.insert({a:[1,2,3]});
	 varCL.insert({a:[2,3,1]});
	 varCL.insert({a:[3,2,1]});
	 varCL.insert({a:[2,4,6,6]});
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find({a:{$in:[1,2,3]}}) ;
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

if ( 4 != size )
{
   println( " get the number of records is wrong.." ) ;
   throw -1 ;
}

var rc1 ;
try
{
   rc1 = varCL.find({a:{$nin:[4]}}) ;
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

if ( 3 != size1 )
{
   println( " get the number of records is wrong...." ) ;
   throw -1 ;
}

var rc2 ;
try
{
   rc2 = varCL.find({a:{$all:[2,1,3]}}) ;
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
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

if ( 3 != size2 )
{
   println( " get the number of records is wrong......" ) ;
   throw -1 ;
}

var rc3 ;
try
{
   rc3 = varCL.find({a:{$all:[6,3]}}) ;
}
catch ( e )
{
   println( "failed to read record, rc3= " + e ) ;
   throw e ;
}

var size3 = 0 ;
while ( true )
{
   var j = rc3.next() ;
   if ( !j )
      break ;
   else
      size3++ ;
}

if ( 0 != size3 )
{
   println( " get the number of records is wrong........" ) ;
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
