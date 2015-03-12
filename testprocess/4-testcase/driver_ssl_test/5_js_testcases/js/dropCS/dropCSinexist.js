//drop not exist collection
CSPREFIX_CS = CSPREFIX+"ONWfoo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
try
{
  db.dropCS(CSPREFIX_CS);
}
catch ( e )
{

}
var res = false;
try{
   db.dropCS(CSPREFIX_CS);
}
catch( e ){
   if(e==-34){
      res = true ;
}
}
if( !res ){
  throw -1;
}