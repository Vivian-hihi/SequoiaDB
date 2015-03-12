// create cs.
// unnormal_2 case.
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
var res = false;
try
{
   db.createCS(CSPREFIX_CS+".cs");
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



