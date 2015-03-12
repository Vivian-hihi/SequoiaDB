// update record.
// update_current delecte_current

csName = COMMCSNAME ;
clName = COMMCLNAME ;

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

try
{

   // Clear the collection space in the beginning
   commDropCL( db, csName, clName, true, true, "Drop CL in the beginning" ) ;
   var varCL = commCreateCL( db, csName, clName, -1, true, true, false,
                             "Create collection in the beginning" ) ;
}
catch( e )
{
   throw e ;
}

try
{
   varCL.insert({a:[1,2],salary:100}) ;
   varCL.insert({a:[1,2],salary:10,name:"Tom"}) ;
}
catch ( e )
{
   println( "failed to insert record, rc = " + e ) ;
   throw e ;
}

try
{
   var rc = varCL.find() ;
   var len = rc.count() ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}


if ( 2 != len )
{
   println( " get the number of records is wrong , len = " + len ) ;
   throw "ErrQueryNum" ;
}
try
{
   var len = varCL.find({salary:10}).count() ;
}
catch ( e )
{
   println( "failed to read record, res= " + e ) ;
   throw e ;
}

if ( 1 != len )
{
   println( " get the number of records is wrong, rc" ) ;
   throw "ErrQueryNum" ;
}

try
{
   // Drop collection space
   commDropCL( db, csName, clName, true, false, "Drop CL in the " ) ;
}
catch ( e )
{
   throw e ;
}
