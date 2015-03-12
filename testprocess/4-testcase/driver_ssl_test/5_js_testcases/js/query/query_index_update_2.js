CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
try
{
   db.dropCS( CSPREFIX_CS) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try{

var claSize = new RSize( CSPREFIX_CS );

var varCS = db.createCS(CSPREFIX_CS);

var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()},{Compressed:true});

}catch( e ){
	println("failed to create CL");
   throw e ;	
}
try
{
   for(var i =0 ;i <1000 ;i++ ){
   	if(i<500){
   		varCL.insert({no:i}) ;
   	}
   	else {
   	varCL.insert({no:i,name:"foo"+i});
   	}
   }
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}
try
{
	varCL.createIndex("testIndex",{name:-1},true);	
}
catch ( e )
{
   println( "failed to create unique index, rc= " + e ) ;
   throw e ;
}
try
{
	rc = varCL.find({$or:[{no:{$gt:250,$lt:500}},{no:{$gt:500,$lt:900}}],name:{$regex:"foo*"},no:{$mod:[5,3]},$and:[{no:{$gt:800}},{no:{$lt:850}}],name:{$nin:["foo898","foo893","foo888"]}});	
}
catch ( e )
{
   println( "failed to find record after create index, rc= " + e ) ;
   throw e ;
}
if(rc.count()!=10){
	println("return wrong number of records after create index");
	println(varCL.find({$or:[{no:{$gt:250,$lt:500}},{no:{$gt:500,$lt:900}}],name:{$regex:"foo*"},no:{$mod:[5,3]},$and:[{no:{$gt:800}},{no:{$lt:850}}],name:{$nin:["foo898","foo893","foo888"]}}));	
	throw -1 ;
}

for(var i =500;i<1000;i++){
	try
	{
		varCL.insert({name:"foo"+i});
	}
	catch ( e )
	{
	   if(e!=-38){
		   println( "it is wrong insert same record about the field of name after create unique index, rc= " + e ) ;
		   throw e ;
	   }
	}
}

	try
	{
		varCL.update({$set:{age:20}},{name:{$exists:0}});
	}
	catch ( e )
	{
		println( "failed to update record , rc= " + e ) ;
		throw e ;
	}

try
{
	rc = varCL.find({age:20});	
}
catch ( e )
{
   println( "failed to find record insert operations, rc= " + e ) ;
   throw e ;
}
if(rc.count()!=500){
	println("return wrong number of records after create same index operation");
	println(varCL.find({age:20}));	
	throw -1 ;
}
try
{
	varCL.dropIndex("testIndex");	
}
catch ( e )
{
   println( "failed to drop unique index, rc= " + e ) ;
   throw e ;
}
try
{
	rc = varCL.find({age:20});	
}
catch ( e )
{
   println( "failed to find record insert operations, rc= " + e ) ;
   throw e ;
}
if(rc.count()!=500){
	println("return wrong number of records after create same index operation");
	println(varCL.find({age:20}));	
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
