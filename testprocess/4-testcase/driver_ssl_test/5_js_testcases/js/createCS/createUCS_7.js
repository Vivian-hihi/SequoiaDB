// create cs.
// CSname's large is 127.
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;

var aa = Array(";","\'","{","}","[","]",",","+","=","-","_","~","`","!","@","#","$","%","^","&","(",")");


var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
for(var i = 0 ; i < aa.length ; ++i ){
        try{
            var CSname = CSPREFIX_CS + aa[i];
            db.dropCS(CSname);
        }catch( e ){
        }
}

var j = 0 ;
var aa = Array(";","\'","{","}","[","]",",","+","=","-","_","~","`","!","@","#","$","%","^","&","(",")");

for(var i = 0 ; i < aa.length ; ++i ){
        try{
            var CSname = CSPREFIX_CS + aa[i];
            db.createCS(CSname);
        }catch( e ){
println(aa[i]);
                        ++j ;
        }
}

if( 0 != j ){
//       throw -1 ;
}

j = 0;

for(var i = 0 ; i < aa.length ; ++i ){
        try{
            var CSname = CSPREFIX_CS + aa[i];
            db.dropCS(CSname);
        }catch( e ){
                        ++j ;
        }
}


if( 0 != j ){
         throw -1 ;
}
