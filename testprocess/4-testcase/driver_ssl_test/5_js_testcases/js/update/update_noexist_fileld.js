// update record.
// normal case. $set
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

var insertCount = 1000;

try
{
  for (i = 0; i <insertCount; i++) 
  {
  	varCL.insert({a:i, b:"fdafdsaf$#@$@%$#%#@!$#@!$", c:null, d:{id:1.0,name:"qiu"},e:{ "$binary" : "aGVsbG8gd29ybGQ=", "$type" : "1" }});
  }
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.update({"$unset":{noexist:""}})
}
catch ( e )
{
   println( "failed to update record, rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find();
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

var recordCount = 0 ;
while ( true )
{
	 var record = eval( "("+ rc.current() +")" );
	 
	 if ( record["a"] != recordCount )
	 {
	 	 println("Record error: the " + i + "th record's a field is not equals " + i);
	 	    throw -1;
	 }
	 
	 recordCount++;
	 
   if ( !rc.next() )
      break ;
}

if ( insertCount != recordCount )
{
   println( "The record count is not equals insert count" ) ;
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



