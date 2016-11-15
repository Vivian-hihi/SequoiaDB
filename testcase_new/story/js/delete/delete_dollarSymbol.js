
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "drop cl in the beginning" ) ;
}
catch ( e )
{
   println("Failed to drop CS ,begin "+e) ;
   throw e ;
}

try
{
   var optionObj = {ReplSize:0,Compressed:true};
   var cl = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, optionObj, true,
                                     false, "create collecton 1 failed" );
}
catch ( e )
{
   println("Failed to create CS and CL "+e) ;
   throw e ;
}

try
{
   cl.insert({a:1}) ;
   cl.insert({"$abc":"china"}) ;
}
catch ( e )
{
   println("Failed to insert record to CL "+e) ;
   throw e ;
}

//the symbol "$" exit in record,remove?insert?
try
{
   cl.remove({a:1}) ;
//cl.remove({"$abc":"china"}) ;
   cl.remove() ;
}
catch ( e )
{
	println("Failed to remove the record "+e) ;
	throw e ;
}

try
{
	var count = cl.count() ;
	if( count!=0 )
	{
		println("Failed to remove records "+count) ;
		throw "Error_Remove" ;
	}
}
catch ( e )
{
	println("Failed remove "+e ) ;
	throw e ;
}

try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop colleciton in the end" );
}
catch ( e )
{
	println("Failed to drop CS ,end "+e) ;
	throw e ;
}



