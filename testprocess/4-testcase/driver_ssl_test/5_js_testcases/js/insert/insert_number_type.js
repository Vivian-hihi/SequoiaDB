// insert record.
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

var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});

}catch( e ){
   throw e ;
}

var records = new Array();

records[0] = {a:0, b:"zero"};
records[1] = {a:-1, b:"plus one"};
records[2] = {a:4294967295, b:"max_int32"};
records[3] = {a:922337203685477587, b:"max_int64"};

for (i = 0; i < records.length; i++)
{
    varCL.insert(records[i]);
}

var cursor = varCL.find();

while (cursor.next())
{
	record = cursor.current();
	recordObj = record.toObj();
	recordStr = record.toJson();
	
	for (i = 0; i < records.length; i++)
	{
		if (recordObj["b"] == records[i]["b"])
		{
			if (recordObj["a"] != records[i]["a"])
			{
				println("the records " + recordStr + " is not match insert records");
				throw -1;
			}
			else
			{
				println("the records " + recordObj + " is match insert records");
				break;
			}
		}
	}
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

