
// list collection.
// normal case.
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
CSPREFIX_CL1 = CSPREFIX+"bar1" ; 
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try
{
   var varCS = db.createCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to create cs, rc= " + e )
   throw e ;
}

try
{
	 var claSize = new RSize( CSPREFIX_CS );
   var varCL = varCS.createCL( CSPREFIX_CL,{ReplSize:claSize.ReplSize(),Compressed:true} ) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e )
   throw e ;
}

try
{
   var varCL1 = varCS.createCL( CSPREFIX_CL1,{ReplSize:claSize.ReplSize(),Compressed:true} ) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e )
   throw e ;
}


try
{
  var cur = db.listCollections() ;
}
catch ( e )
{
   println( "failed to list collections, rc= " + e ) ;
   throw e ;
}

while(cur.next())
{
   
   var strCSAndCLName = cur.current().toObj()["Name"]
   var strCS = strCSAndCLName.substring(0,strCSAndCLName.indexOf("."));
   var strCL = strCSAndCLName.substring(strCSAndCLName.indexOf(".") + 1);
   
   if ( strCS == CSPREFIX_CS && (strCL != CSPREFIX_CL && strCL != CSPREFIX_CL1) )
   {
      println( "uncorrect name in list: " + cur.current().toObj()["Name"] )
      throw -1 ;
   }
}

try
{
   varCS.dropCL( CSPREFIX_CL ) ;
}
catch ( e )
{
   println( "failed to drop collections, rc= " + e ) ;
   throw e ;
}

try
{
   cur = db.listCollections() ;
}
catch ( e )
{
   println( "failed to list collections, rc= " + e ) ;
   throw e ;
}


while(cur.next())
{
   
   
   var strCSAndCLName = cur.current().toObj()["Name"];
   var strCS = strCSAndCLName.substring(0,strCSAndCLName.indexOf("."));
   var strCL = strCSAndCLName.substring(strCSAndCLName.indexOf(".") + 1);
   
   if ( strCS == CSPREFIX_CS && strCL != CSPREFIX_CL1 )
   {
      println( "uncorrect name in list: " + o["Name"] )
      throw -1 ;
   }
}

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}
