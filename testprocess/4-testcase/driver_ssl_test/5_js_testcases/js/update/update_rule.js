// update record.
// unnormal rule. 
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
   println( "Create collection: " + CSPREFIX_CL + "failed: " + e ) ;
   throw e ;	
}
try
{
   varCL.insert({a:[1,2],salary:100}) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}
var res = false ;
try
{
   varCL.update( {$addtoset:{b:2}}) ;
}
catch ( e )
{ 
	 if ( e == -6 )
	 {
        res = true ;
	 }
	 else
	 {
	    println( "Update {$addtoset:{b:2}} failed: " + e ) ;
	 }
}
if ( !res )
{
  throw -1	
}

var res1 = true ;
try
{
   varCL.update( {$pull:{"a.0":1}}) ;
}
catch ( e )
{
   res1 = false ;
   println( "Update {$pull:{a.0:1}} failed: " + e ) ;
}
if ( !res1 )
{
  throw -1	
}

var res2 = true ;
try
{
   varCL.update( {$push:{salary:1}}) ;
}
catch ( e )
{ 
   res2 = false ;
   println( "Update {$push:{salary:1}} failed: " + e ) ;
}
if ( !res2 )
{
  throw -1	
}
var res3 = false ;
try
{
   varCL.update( {$pull_all:{a:3}}) ;
}
catch ( e )
{ 
	 if ( e == -6 )
	 {
        res3 = true ;
	 }
	 else
	 {
	    println( "Update {$pull_all:{a:3}} failed: " + e ) ;
	 }
}
if ( !res3 )
{
  throw -1	
}
var res4 = false ;
try
{
   varCL.update( {$push_all:{a:2}}) ;
}
catch ( e )
{ 
	 if ( e == -6 )
	 {
        res4 = true ;
	 }
	 else
	 {
	    println( "Update {$push_all:{a:2}} failed: " + e ) ;
	 }
}
if ( !res4 )
{
  throw -1	
}
var res5 = false ;
try
{
   varCL.update( {$pop:{a:[2]}}) ;
}
catch ( e )
{ 
	 if ( e == -6 )
	 {
        res5 = true ;
	 }
	 else
	 {
	    println( "Update {$pop:{a:[2]}} failed: " + e ) ;
	 }
}
if ( !res5 )
{
  throw -1	
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


