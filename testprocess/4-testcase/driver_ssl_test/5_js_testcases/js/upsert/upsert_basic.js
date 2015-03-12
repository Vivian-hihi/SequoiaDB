// upsert record.
// normal case.
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ; 
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

// clear
commDropCS( db, CSPREFIX_CS, true, "clear in begin" ) ;

// create cs, cl
var varCL = commCreateCL( db, CSPREFIX_CS, CSPREFIX_CL, -1, true, true, false, "create cs and cl in begin" ) ;

try
{
   varCL.insert({a:1}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.upsert( {$set:{a:2}}, {a:1}) ;
}
catch ( e )
{
   println( "failed to update( {$set:{a:2}}, {a:1}) record, rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find({a:2}) ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

var size = rc.size();
if ( 1 != size )
{
   println( " get more than one record after upsert($set:{a:2}}, {a:1})" ) ;
   throw -1 ;
}

try
{
   varCL.upsert( {$set:{a:4}}, {a:3}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

try
{
   rc = varCL.find() ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

size = 0;

var bAddNew = false;
while(rc.next())
{
	var recordObj = rc.current().toObj();
	var recordStr = rc.current().toJson();
	if (recordObj["a"] == 4)
	{
		bAddNew = true;
	}
	size++;
}

if (size != 2 || !bAddNew)
{
	println("The size is not equal 2 or The new record no be added. size=" + size);
	println(varCL.find());
	throw -1;
}

// clear
commDropCS( db, CSPREFIX_CS, false, "clear in end" ) ;

