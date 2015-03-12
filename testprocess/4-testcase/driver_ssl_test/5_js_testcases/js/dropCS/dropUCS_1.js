//drop innomal collection space 

var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
var res = false;
try{
   db.dropCS("$dropUCS");
}
catch( e ){
   if(e==-34){
      res = true ;
}
}
if( !res ){
  throw -1;
}