//use sql drop CL
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

//clear environment
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
//create CS
try
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS);
}
catch(e)
{
  println("failed to create CS , rc =" +e);
  throw e ;	
}
//create CL
try 
{
	db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
	println("failed to create collection CL , rc = "+e);
	throw e ;
}
//drop CL
try
{
	db.execUpdate("drop collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch(e)
{
	println("failed to drop CL , rc ="+e);	
	throw e ;
}

//drop again
var res = false ;
try
{
	db.execUpdate("drop collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch(e)
{
	if( e == -23)
	{
	   res = true ;
	}
	else
	{
	   println( "drop collection " + CSPREFIX_CS + "." + CSPREFIX_CL + " failed: " + e ) ;
	}
}
if(!res)
{
   println( "failed1" ) ;
   throw -1 ;
}
//drop a CL,the CL is a empty string
var res = false ;
try
{
	db.execUpdate("drop collection "+CSPREFIX_CS+"."+"");	
}
catch(e)
{
	if( e == -6)
	{
	   res = true ;
	}
	else
	{
	   println( "drop collection " + CSPREFIX_CS + "." + "" + " failed: " + e ) ;
	}
}
if(!res)
{
   println( "failed2" ) ;
   throw -1 ;	
}
//drop illegal named cl
var res = false ;
var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*","(",")","SYS");
for(var i = 0 ; i < aa.length ; ++i ){
   	try{
   	    var CLname = aa[i] + CSPREFIX ;	
   	    db.execUpdate("drop collection "+CSPREFIX_CS+"."+CLname);
   	}catch( e ){
   			if(e == -23)
			{
   			   res = true ;
			}
			else
			{
			   println( "drop collection " + CSPREFIX_CS + "." + CLname + " failed: " + e ) ;
			}
   	}
}

if( !res ){
   println( "failed3" ) ;
   throw -1 ; 
}

//drop a CL,it's length is more than 127B
var CL = "";
for(var i = 0 ; i < 128 ; ++i ){
	  CL = CL+"a";
	}
var res = false ; 
try
{
   db.execUpdate( "drop collection "+CSPREFIX_CS+"."+CL) ;
}
catch ( e )
{
   // when standalone, result is -6, because name length must <= 127
   // when cluster, coord query to catalog return -23
   if ( e == -23 || e == -6 )
   {
      res = true ;
   }
   else
   {
      println( "drop collection " + CSPREFIX_CS + "." + CL + " failed: " + e ) ;
   }
}
if( !res ){
   println( "failed4" ) ;
   throw -1 ; 
}
//drop a CL,it's length is 127B
for(var i = 0 ; i < 127-CSPREFIX.length-3  ; ++i ){
	  CSPREFIX_CL = CSPREFIX_CL+"a";
	}
try
{
   db.execUpdate( "create collection "+CSPREFIX_CS+"."+CSPREFIX_CL) ;
}
catch ( e )
{
   println("fail to create CL " +  CSPREFIX_CS + "." + CSPREFIX_CL + " failed: " + e );
   throw e ;
}

try
{
   db.execUpdate( "drop collection "+CSPREFIX_CS+"."+CSPREFIX_CL) ;
}
catch ( e )
{
   println("failed to drop cl " + CSPREFIX_CS + "." + CSPREFIX_CL + " failed: " + e );
   throw e ;
}

try
{
  db.dropCS(CSPREFIX_CS);
}
catch(e)
{
  println( "unexpected err happened when clear cs:" + e ) ;
  throw e ;
}
