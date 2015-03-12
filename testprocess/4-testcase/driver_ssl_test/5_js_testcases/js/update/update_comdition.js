// update record.
// update_condition rule. 
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
   varCL.insert({a:[1,2],salary:100}) ;
   varCL.insert({a:[1,2],salary:10,name:"Tom"}) ;
   varCL.insert({a:[1,2],salary:10,name:"Mike",age:20}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.update({$set:{age:25}},{age:{$exists:1}}) ;
}
catch ( e )
{ 
	 println("failed to update records,rc");
	 throw e ;
}

var rc ;
try
{
   rc = varCL.find({age:25});
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

if (1 != size )
{
   println( " get more than one record, rc" ) ;
   throw -1 ;
}

try
{
   varCL.update({$set:{age:30}},null) ;
}
catch ( e )
{ 
	 println("failed to update records,rc1");
	 throw e ;
}

var rc1 ;
try
{
   rc1 = varCL.find({age:30});
}
catch ( e )
{
   println( "failed to read record, rc1= " + e ) ;
   throw e ;
}

var size1 = 0 ;
while ( true )
{
   var i = rc1.next() ;
   if ( !i )
      break ;
   else
      size1++ ;
}

if (3 != size1 )
{
   println( " get more than one record, rc1" ) ;
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



