// upsert record.
// normal case.
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

// clear
commDropCS( db, CSPREFIX_CS, true, "clear in begin" ) ;

// create cs, cl
var varCL = commCreateCL( db, CSPREFIX_CS, CSPREFIX_CL, -1, true, true, false, "create cs and cl in begin" ) ;
var count = 100 ;

try
{
   for (i = 0; i < count; i++)
   {
      varCL.insert({id:i, mineName:"上海矿场",localtion:{resId:0, resourceName:null, country:"中国", state:"黑龙江", city:"佳木斯市"}}) ;
   }
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}

try
{
   varCL.upsert( {$set:{"localtion.street":"人民路12号"}} , {mineTime:{$et:"2013-06-14"}} ) ;
}
catch ( e )
{
   println( "failed to update( {$set:{localtion.street:人民路12号}} , {mineTime:2013-06-14} , rc= " + e ) ;
   throw e ;
}

var rc ;
try
{
   rc = varCL.find({localtion:{$elemMatch:{street:"人民路12号"}}}) ;
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

var size = 0 ;

while(rc.next())
{
   recordObj = rc.current().toObj();
   recordStr = rc.current().toJson();
   if (recordObj["mineName"] != undefined )
   {
      println("The record error, because of include mineName:" +  recordStr);
      throw -1 ;
   }
   size++;
}

if (size != 1)
{
  println("The record size of not equal " + count );
  println( varCL.find() );
  throw -1;
}

// clear
commDropCS( db, CSPREFIX_CS, false, "clear in end" ) ;

