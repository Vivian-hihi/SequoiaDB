CSPREFIX_CS = CSPREFIX+"ONEfoo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
try
{
  db.dropCS(CSPREFIX_CS);
}
catch(e)
{
  if ( e != -34)
    {
        println( "unexpected err happened when clear cs:" + e ) ;
        throw e ;
     }
}
try
{
  var varCS = db.createCS(CSPREFIX_CS);
}
catch(e)
{
  println("failed to create cs,rc="+e);
  throw e;
}
try
{
  db.dropCS(CSPREFIX_CS);
}
catch(e)
{

}

var res = false;
try
{
  db.dropCS(CSPREFIX_CS);
}
catch(e)
{
  if ( e == -34){ res = true;}
}  
if(!res) { throw -1;}