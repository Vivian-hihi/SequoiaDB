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
//insert 7 records
try
{
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Tom\","+1+")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Mike\","+2+")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Lisa\","+3+")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Json\","+4+")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Jhon\","+5+")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Tina\","+6+")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Pite\","+7+")");
}
catch(e)
{
	println("failed to insert records , rc ="+e);
	throw e ;
}
//delete records satisfy the condition
try
{
	db.execUpdate("delete from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age=3");
}
catch(e)
{
	println("failed to delete the record , rc ="+e);
	throw e ;
}

//delete  all the records 
try
{
	db.execUpdate("delete from "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
	println("failed to delete the record , rc ="+e);
	throw e ;
}
//clear environment
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
