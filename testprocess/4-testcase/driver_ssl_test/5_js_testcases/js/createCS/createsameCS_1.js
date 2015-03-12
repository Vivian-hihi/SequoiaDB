//creat same collection space
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
try
{
   db.dropCS(CSPREFIX_CS);
}
catch ( e )
{

}
try
{
  db.createCS(CSPREFIX_CS);
}
catch ( e )
{
  println("failed to create cs,rc="+e);
  throw e ;
}

var res = false ;
try
{
   db.createCS(CSPREFIX_CS);
}
catch (e)
{
   if ( e==-33) res = true ;
}

if (!res) {throw -1 ;}

try
{
   db.dropCS( CSPREFIX_CS );
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}