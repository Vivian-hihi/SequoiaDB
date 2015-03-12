// create cs.
// unnormal_1 case. 
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
var res = false;
try
{
   db.createCS("SYS"+CSPREFIX_CS);
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
try
{
   db.dropCS("SYS"+CSPREFIX_CS);
}
catch( e )
{ 
}

