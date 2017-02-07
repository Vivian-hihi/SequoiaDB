/************************************************************************
*@Description:   seqDB-11052:jstobs支持$code
*@Author:  2017/2/7  huangxiaoni
************************************************************************/
main();

function main()
{  
   try
   {
      //ready env
      cleanEnv( true );
      
      //structural data
      println("\n---Begin to createProcedure.");
      db.createProcedure( function abc11502(x,y) { return x+y; } );
      
      checkResult();
      
      //clean env
      cleanEnv( false );
   }
      catch(e)
   {
   	throw e;
   }
}

function checkResult()
{
   println("\n---Begin to check result.");
   
   //compare the returned records
   var rc = db.listProcedures({name:"abc11502"}).current().toObj().func;
   
   var expRlt = '{"$code":"function abc11502(x, y) {\\n    return x + y;\\n}"}';
   var actRlt = JSON.stringify( rc );
   if( expRlt !== actRlt )
   {
      throw buildException("checkResult", null, "[compare the records]", 
                          "["+ expRlt +"]",
                          "["+ actRlt +"]");
   }
}

function cleanEnv( ignoreExisted )
{
   try
   {
      db.removeProcedure("abc11502");
   }
   catch( e )
   {
      if ( e !== -233 || !ignoreExisted )
      {
         throw e ;
      }
   }
}