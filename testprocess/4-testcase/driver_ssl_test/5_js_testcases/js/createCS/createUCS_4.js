// create cs
// unnormal case
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
var res = false;
var name = "";
for(var i = 0 ; i < 10000; i++)
{
   name = name + "a";	
}
try
{
   db.createCS(name);
}
catch( e )
{ 
   if(e==-6){
      res = true;
   }
}
if( !res ){
  throw -1;
}
