//use sql create normal CL 

CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;  
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

//create CL in the CS which isnot exist.
var res = false ;
try
{
   //db.execUpdate( "create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);
   
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.getCS(CSPREFIX_CS);
   var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch ( e )
{
	if( e == -34 )
   res = true ;
}
if ( !res )
{
	throw -1 ;
	}

//first create CS,then create CL in the CS
try 
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS) ;
}
catch( e )
{
	println ("failed to create CS , rc1 = "+e);
	throw e ;
}
try
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch( e )
{
	println ("failed to create CL , rc2 = "+e);
	throw e ;
}

//create the same CL again 
var res = false ;
try
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch( e )
{
	if ( e==-22 )
	res =  true ;
}
if ( !res )
{
 throw -1 ;	
}

//first drop the cl , then create it again
try 
{
	db.execUpdate("drop collection "+CSPREFIX_CS+"."+CSPREFIX_CL) ;
	}
catch ( e )
{
	println("fail to clear cl , rc ="+e);
	throw e ;
	}
try
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch( e )
{
	println ("failed to create CL , rc2 = "+e);
	throw e ;
}
	
//create same CL in another CS
CSPREFIX_CS1 = CSPREFIX+"foo1" ;

try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS1 ) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}
try 
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS1) ;
}
catch( e )
{
	println ("failed to create CS , rc4 = "+e);
	throw e ;
}
try
{
  db.execUpdate("create collection "+CSPREFIX_CS1+"."+CSPREFIX_CL);	
}
catch( e )
{
	println ("failed to create CL , rc5 = "+e);
	throw e ;
}

try
{
		db.execUpdate("drop collectionspace "+CSPREFIX_CS1);	
}
catch(e)
{
	println("failed to drop CS1,rc = "+e);
	throw e ;
}

//create a cl and it's length is 128B
var cl = "" ;
for(var i = 0 ; i < 128  ; ++i ){
	  cl = cl+"a";
	}
var res = false ; 
try
{
   db.execUpdate( "create collection "+CSPREFIX_CS+"."+cl) ;
}
catch ( e )
{
   if ( e == -6) 
   res = true ;
}
if( !res ){
   throw -1 ; 
}

//create a cl and it's length is 127B
for(var i = 0 ; i < 127-CSPREFIX.length-3  ; ++i ){
	  CSPREFIX_CL = CSPREFIX_CL+"a";
	}
try
{
   db.execUpdate( "create collection "+CSPREFIX_CS+"."+CSPREFIX_CL) ;
}
catch ( e )
{
   println("fail to create CL , rc = "+ e);
   throw e ;
}

try
{
		db.execUpdate("drop collectionspace "+CSPREFIX_CS);	
}
catch(e)
{
	println("failed to drop CS,rc = "+e);
	throw e ;
}
