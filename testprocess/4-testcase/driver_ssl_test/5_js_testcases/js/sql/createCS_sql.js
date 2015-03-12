//use SQL create normal cs

CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;

var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

//clear environment , if the CS "foo" is exists ,first clear it
try
{
   db.execUpdate("drop collectionspace "+CSPREFIX_CS) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

//create cs "foo" for the first time.
try
{
   db.execUpdate( "create collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to create cs, rc= " + e );
   throw e ;
}

try
{
   var rc = db.getCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to get cs for the first time, rc= " + e );
   throw e ;
}

//create cs "foo" for the second time , then we will get the exception -33.
var res = false ;
try
{
	db.execUpdate( "create collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
	if ( e == -33 ){
		res = true ;
		}
}
if (!res) {
	throw -1 ;
	}

//after drop cs "foo" , if create cs "foo" will return err.
try
{
   db.execUpdate("drop collectionspace "+CSPREFIX_CS) ;
}
catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

try
{
   db.execUpdate( "create collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to create cs, rc= " + e );
   throw e ;
}

try
{
   var rc = db.getCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to get cs for the first time, rc= " + e );
   throw e ;
}

//test is over,clear environment again
try
{
   db.execUpdate("drop collectionspace "+CSPREFIX_CS) ;
}
catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}
