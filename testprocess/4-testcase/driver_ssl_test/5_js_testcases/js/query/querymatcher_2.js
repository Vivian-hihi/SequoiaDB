// query record.
// matcher $type

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
	 varCL.insert({doubletype:56.2321});//$type=1
   varCL.insert({stringtype:"Tom"});//$type=2
   varCL.insert({arraytype:["123","456"]});//$type=4
   varCL.insert({timetype:{$timestamp:"2013-03-05-10.22.10.123456"}});//$type=17
   varCL.insert({datetype:{$date:"2013-03-04"}});//$type=9
   varCL.insert({regextype:{$regex:"^a",$options:"i"}});//$type=11
   varCL.insert({booltype:false});//$type=8
   varCL.insert({oidtype:{$oid:"456465487983454624jhiuyt"}});//$type=7
   varCL.insert({longtype:3147483689});//$type=18
   varCL.insert({inttype:483689});//$type=16
   
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}
var rc1;
try
{
   rc1 = varCL.find({doubletype:{$type:1}}) ;
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
   println( " get the number of doubletype records is wrong" ) ;
   throw -1 ;
}

var rc2 ;
try
{
   rc2 = varCL.find({stringtype:{$type:2}}) ;
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
   println( " get the number of stringtype records is wrong" ) ;
   throw -1 ;
}

var rc3 ;
try
{
   rc3 = varCL.find({arraytype:{$type:4}}) ;
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

if ( 1 != size3 )
{
   println( " get the number of arraytype records is wrong" ) ;
   throw -1 ;
}

var rc4 ;
try
{
   rc4 = varCL.find({timetype:{$type:17}}) ;
}
catch ( e )
{
   println( "failed to read record, rc4= " + e ) ;
   throw e ;
}
var size4 = 0 ;
while ( true )
{
   var j = rc4.next() ;
   if ( !j )
      break ;
   else
      size4++ ;
}

if ( 1 != size4 )
{
   println( " get the number of timetype records is wrong.................." ) ;
   throw -1 ;
}

var rc5 ;
try
{
   rc5 = varCL.find({datetype:{$type:9}}) ;
}
catch ( e )
{
   println( "failed to read record, rc5= " + e ) ;
   throw e ;
}
var size5 = 0 ;
while ( true )
{
   var j = rc5.next() ;
   if ( !j )
      break ;
   else
      size5++ ;
}

if ( 1 != size5 )
{
   println( " get the number of datetype records is wrong" ) ;
   throw -1 ;
}

var rc6;
try
{
   rc6 = varCL.find({regextype:{$type:11}}) ;
}
catch ( e )
{
   println( "failed to read record, rc6= " + e ) ;
   throw e ;
}
var size6 = 0 ;
while ( true )
{
   var j = rc6.next() ;
   if ( !j )
      break ;
   else
      size6++ ;
}

if ( 1 != size6 )
{
   println( " get the number of regextype records is wrong" ) ;
   throw -1 ;
}
 
var rc7 ;
try
{
   rc7 = varCL.find({booltype:{$type:8}}) ;
}
catch ( e )
{
   println( "failed to read record, rc7= " + e ) ;
   throw e ;
}
var size7 = 0 ;
while ( true )
{
   var j = rc7.next() ;
   if ( !j )
      break ;
   else
      size7++ ;
}

if ( 1 != size7 )
{
   println( " get the number of booltype records is wrong" ) ;
   throw -1 ;
}

var rc8 ;
try
{
   rc8 = varCL.find({oidtype:{$type:7}}) ;
}
catch ( e )
{
   println( "failed to read record, rc8= " + e ) ;
   throw e ;
}
var size8 = 0 ;
while ( true )
{
   var j = rc8.next() ;
   if ( !j )
      break ;
   else
      size8++ ;
}

if ( 1 != size8 )
{
   println( " get the number of oidtype records is wrong" ) ;
   throw -1 ;
}
/*
var rc9 ;
try
{
   rc9 = varCL.find({longtype:{$type:18}}) ;
}
catch ( e )
{
   println( "failed to read record, rc9= " + e ) ;
   throw e ;
}
var size9 = 0 ;
while ( true )
{
   var j = rc9.next() ;
   if ( !j )
      break ;
   else
      size9++ ;
}

if ( 1 != size9 )
{
   println( " get the number of longtype records is wrong" ) ;
   throw -1 ;
}
*/
var rc10 ;
try
{
   rc10 = varCL.find({inttype:{$type:16}}) ;
}
catch ( e )
{
   println( "failed to read record, rc10= " + e ) ;
   throw e ;
}
var size10 = 0 ;
while ( true )
{
   var j = rc10.next() ;
   if ( !j )
      break ;
   else
      size10++ ;
}

if ( 1 != size10 )
{
   println( " get the number of inttype records is wrong" ) ;
   throw -1 ;
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

if ( 10 != size )
{
   println( " get the number of records is wrong" ) ;
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
