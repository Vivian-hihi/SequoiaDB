// create cs.
// unnormal_3 case
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
var res = false;
try
{
   db.createCS("");
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



