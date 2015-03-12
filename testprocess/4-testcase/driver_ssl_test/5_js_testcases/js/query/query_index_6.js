
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

// clear
commDropCS( db, CSPREFIX_CS, true, "clear in begin" ) ;

// create cs, cl
var varCL = commCreateCL( db, CSPREFIX_CS, CSPREFIX_CL, -1, true, true, false, "create cs and cl in begin" ) ;

try
{
   for( var i =0 ;i <1000 ;i++ )
   {
      if( i<500 )
      {
         varCL.insert({no:i}) ;
      }
      else
      {
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

var count = 0 ;
try
{
   count = rc.count() ;
   println( "count1: " + count ) ;
}
catch( e )
{
   println( "get rc count after create index failed: " + e ) ;
   throw e ;
}

if( count != 10 )
{
   println("return wrong number of records after create index");
   println(varCL.find({$or:[{no:{$gt:250,$lt:500}},{no:{$gt:500,$lt:900}}],name:{$regex:"foo*"},no:{$mod:[5,3]},$and:[{no:{$gt:800}},{no:{$lt:850}}],name:{$nin:["foo898","foo893","foo888"]}}));   
   throw -1 ;
}

try
{
   varCL.createIndex("testIndex",{name:-1},{isUnique:true});   
}
catch ( e )
{
   if( e!=-247 )
   {
      println( "it is wrong can create same indexname, rc= " + e ) ;
      throw e ;
   }
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

try
{
   count = rc.count() ;
   println( "count2: " + count ) ;
}
catch( e )
{
   println( "get rc count after re-create index failed: " + e ) ;
   throw e ;
}

if( count != 10 )
{
   println("return wrong number of records after create same index operation");
   println(varCL.find({$or:[{no:{$gt:250,$lt:500}},{no:{$gt:500,$lt:900}}],name:{$regex:"foo*"},no:{$mod:[5,3]},$and:[{no:{$gt:800}},{no:{$lt:850}}],name:{$nin:["foo898","foo893","foo888"]}}));   
   throw -1 ;
}

// clear
commDropCS( db, CSPREFIX_CS, false, "clear in end" ) ;

