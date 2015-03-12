//use sql create unnormal CL
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

//the name of CL start with $
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
try
{
  db.execUpdate("create collectionspace "+CSPREFIX_CS);
}
catch ( e )
{
  println("failed to create cs,rc="+ e );
  throw e ;
}

var res = false ;
 try 
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+"$cl");
}
catch( e )
{
  if ( e == -6)
    {
      res = true ;
     }
}
if ( !res )
{
  throw -1;
}

//the name of CL start with SYS
var res = false ;
 try 
{
  db.execUpdate("create collection "+CSPREFIX_CS+".SYScl");
}
catch( e )
{
  if ( e == -6)
    {
      res = true ;
     }
}
if ( !res )
{
  throw -2;
}

//the name of CL contain "."
var res = false ;
 try 
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+"c.l");
}
catch( e )
{
  if ( e == -6)
    {
      res = true ;
     }
}
if ( !res )
{
  throw -3;
}

//the name of CL is a empty string
var res = false ;
 try 
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+" ");
}
catch( e )
{
  if ( e == -6)
    {
      res = true ;
     }
}
if ( !res )
{
  throw -4;
}


var res = false ;
var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*","(",")");
for(var i = 0 ; i < aa.length ; ++i ){
   	try{
   	    var CLname = aa[i] + CSPREFIX ;	
   	    db.execUpdate("create collection "+CSPREFIX_CS+"."+CLname);
   	}catch( e ){
   			if(e == -6)
   			res = true ;
   	}
}

if( !res ){
   throw -5 ; 
}
