
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

//update all the records:add a field,it's name is phone and value is 123
try
{
  db.execUpdate("update "+CSPREFIX_CS+"."+CSPREFIX_CL+" set phone=123");   
}
catch(e)
{
   println("failed to update the records , rc ="+e);
   throw e ;
}
//update one record
try
{
  db.execUpdate("update "+CSPREFIX_CS+"."+CSPREFIX_CL+" set age=10 where name=\"Lisa\"");   
}
catch(e)
{
   println("failed to update the record , rc ="+e);
   throw e ;
}
var rc ;
try
{
   rc=db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where age=10")   
}
catch(e)
{
   println("failed to read the record age=10 , rc="+e);
   throw e ;   
}
if( 1 != rc.size() )
{
   println("get more than one records");
}

//update records satisfy the condition
try
{
   db.execUpdate("update "+CSPREFIX_CS+"."+CSPREFIX_CL+" set phone=456 where age>4");   
}
catch(e)
{
   println("failed to update the record , rc ="+e);
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
