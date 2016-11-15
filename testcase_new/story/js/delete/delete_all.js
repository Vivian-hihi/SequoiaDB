// delete record.
// normal case.
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "drop cl in the beginning" ) ;
}
catch ( e )
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

try
{
   var optionObj = {ReplSize:0,Compressed:true};
   var varCL = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, optionObj, true,
                                     false, "create collecton 1 failed" );
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

var size = 0;
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
if ( 2 != size )
{
   println( " get the nnumber of records is wrong" ) ;
   throw -1 ;
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
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop colleciton in the end" );
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}

