
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

function main( db )
{
   // Judge transaction is enable ?
   if ( commIsTransEnabled( db ) == false )
   {
      println( "transaction is not enabled" ) ;
      db.close() ;
      return ;
   }

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
   for ( var i = 0 ; i<20 ; i++)
   {
      try
      {
         db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (age) values ("+i+")");
      }
      catch(e)
      {
         println("failed to insert records , rc ="+e);
         throw e ;
      }
   }
      
   try
   {
      var cs = db.getCS(CSPREFIX_CS);
      var cl = cs.getCL(CSPREFIX_CL);
   }
   catch(e)
   {
      println("failed to get cs or cl ,rc="+e);
      throw e ;
   }

   //start transaction
   try
   {
      db.transBegin();   
   }
   catch(e)
   {
      println("err happend when start transaction,rc="+e);
      throw e ;
   }
   //database operations
   try
   {
      db.execUpdate("update "+CSPREFIX_CS+"."+CSPREFIX_CL+" set name=\"Tom\" where age=19");
      //cl.update({$set:{name:"Tom"}},{age:19});
   }
   catch(e)
   {
      println("failed to update the record age=19,rc="+e);
      throw e ;   
   }

   var rc ;
   try
   {
      rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where name=\"Tom\"");
   }
   catch ( e )
   {
      println( "failed to read record name=Tom, rc= " + e ) ;
      throw e ;
   }

   if ( rc.size()!=1 )
   {
      println( " get the record name=Tom happend err" ) ;
      throw -1;
   }
   try
   {
      db.transCommit();   
   }
   catch(e)
   {
      println("failed to commit the transcation,rc="+e);
      throw e ;   
   }

   var rc ;
   try
   {
      rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where name=\"Tom\"");
   }
   catch ( e )
   {
      println( "failed to read record name=Tom, rc= " + e ) ;
      throw e ;
   }

   if ( rc.size()!=1 )
   {
      println( " update operation execute failure after the transcation commit" ) ;
      throw -1;
   }

   //clear enviroment 
   try
   {
      db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
   }
   catch (e)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

main( db ) ;
