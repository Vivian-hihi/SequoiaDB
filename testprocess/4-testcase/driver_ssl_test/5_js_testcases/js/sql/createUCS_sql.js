//use SQL create cs.
// unnormal case. 
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;

var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

try
{
	db.execUpdate("drop collectionspace "+CSPREFIX_CS);
}
catch(e)
{
	if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}


var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*","(",")","SYS",".");
for(var i = 0 ; i < aa.length ; ++i ){
        try{
            var CSname = aa[i] + CSPREFIX ;
            db.execUpdate("drop collectionspace   "+CSname);
        }catch( e ){
        }
}


//the name of cs cannot empty string
var res = false;
try
{
   db.execUpdate("create collectionspace "+" ");
}
catch( e )
{ 
   if(e==-195){
      res = true;
   }
}
if( !res ){
  throw -1;
}


var res = false ;
var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*","(",")","SYS",".");
for(var i = 0 ; i < aa.length ; ++i ){
   	try{
   	    var CSname = aa[i] + CSPREFIX ;	
   	    db.execUpdate("create collectionspace "+CSname);
   	}catch( e ){
   			if(e == -6)
   			res = true ;
   	}
}

if( !res ){
   throw -2 ; 
}

//the name of cs's length is 128B
var cs = CSPREFIX_CS;

for(var i = 0 ; i < 128-CSPREFIX_CS.length; ++i ){
   cs = cs +"a";	
}
var res = false;
try
{
   db.execUpdate( "create collectionspace "+cs);
}
catch( e )
{ 
   if ( e == -6)
   res = true;

}
if( !res ){
  throw -3;
}

//the cs's length is 127B
for(var i = 0 ; i < 127-CSPREFIX_CS.length; ++i ){
   CSPREFIX_CS = CSPREFIX_CS+"a";	
}



try{ db.dropCS(CSPREFIX_CS); }catch( e ){ }

try
{
    db.execUpdate( "create collectionspace "+CSPREFIX_CS );
}
catch( e )
{ 
    println( "unexpected err happened when create cs:" + e ) ;
    throw e;
}

try
{
	db.execUpdate("drop collectionspace "+CSPREFIX_CS);
}
catch(e)
{
	println("failed to drop 127B CS, rc="+e);
	throw e ;	
}
var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*","(",")","SYS",".");
for(var i = 0 ; i < aa.length ; ++i ){
        try{
            var CSname = aa[i] + CSPREFIX ;
            db.execUpdate("drop collectionspace   "+CSname);
        }catch( e ){
        }
}
