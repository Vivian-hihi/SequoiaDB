CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
//clear environment
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
//create CS
try 
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS) ;
}
catch( e )
{
	println ("failed to create CS , rc1 = "+e);
	throw e ;
}
//create CL.
try
{
  //db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
  var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch( e )
{
	println ("failed to create CL , rc2 = "+e);
	throw e ;
}
//insert 20 records
for ( var i = 0 ; i<20 ; i++){
	try{
		db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Tom\","+i+")");
		}
		catch(e){
			println("failed to insert records , rc ="+e);
			throw e ;
			}
	}
//select * from table 
var rc ;
try
{
	rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 20 != rc.size() )
{
   println( " get more than 20 records " ) ;
   throw -1;
}

//select age from table 
var rc ;
try
{
	rc = db.exec("select age from "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}
if ( 20 != rc.size() )
{
   println( " get more than 20 records .." ) ;
   throw -1 ;
}
//select * from table where age>10
var rc ;
try
{
	rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age>10");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 9 != rc.size() )
{
   println( " get more than 9 records..." ) ;
   throw -1 ;
}

//select _id,age from table where age>10
var rc ;
try
{
	rc = db.exec("select _id,age from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age>10");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 9 != rc.size() )
{
   println( " get more than 9 records ...." ) ;
   throw -3 ;
}
//select name ,age from table where age=10
var rc ;
try
{
	rc = db.exec("select name,age from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age=10");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 1 != rc.size() )
{
   println( " get more than one records ....." ) ;
   throw -4 ;
}
//select name,age from table where age<10
var rc ;
try
{
	rc = db.exec("select name,age from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age<10");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 10 != rc.size() )
{
   println( " get more than 10 records .." ) ;
   throw -5 ;
}
//select name,age from table where age>=10
var rc ;
try
{
	rc = db.exec("select name,age from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age>=10");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 10 != rc.size() )
{
   println( " get more than 10 records....." ) ;
   throw -6 ;
}
//select name,age from table where age<=10
var rc ;
try
{
	rc = db.exec("select name,age from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age<=10");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 11 != rc.size() )
{
   println( " get more than 11 records .." ) ;
   throw -7 ;
}

//select * from table where name = "Tom" and age !=5
var rc ;
try
{
	rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where name=\"Tom\" and age<>5");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 19 != rc.size() )
{
   println( " get more than 19 records .."+rc.size() ) ;
   throw -8 ;
}

//select * from table where age>10 and age<15 order by age desc
var rc ;
try
{
	rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age>10 and age<15 order by age desc");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 4 != rc.size() )
{
   println( " get more than 4 records ...." ) ;
   throw -9 ;
}

//select * from table where age<5 or age>15 order by asc
var rc ;
try
{
	rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age<5 or age>15 order by age asc");
}
	catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}

if ( 9 != rc.size() )
{
   println( " get more than 9 records ......." ) ;
   throw -10 ;
}

//clear environment
try
{
  // db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch (e)
{
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e 
}
