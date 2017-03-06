/****************************************************
@description: seqDB-11201:query.hint.count查询
@author:
              2017-3-7 huangxiaoni init
****************************************************/
main();

function main()
{  
   try
   {
      db.setSessionAttr( { PreferedInstance: "M" } );
      
      var clName  = COMMCLNAME+"_11201";
      var idxName = "idx";
      
      var cl = readyCL( clName );
      cl.createIndex( idxName, {a:-1} );
      insertRecs( cl );
      var rc = queryRecs( cl, idxName );
      checkResult( rc );
   
      cleanCL( clName );
   }
      catch(e)
   {
   	throw e;
   }
}

function insertRecs( cl )
{
   println("\n---Begin to insert records.");
   
   for( i = 0 ; i < 50 ; i++ )
   {
      cl.insert({a:i,b:i});
   }
}

function queryRecs( cl, idxName )
{
   println("\n---Begin to exec[query.hint.count].");
   
   var rc = cl.find({b:{$gte:10}}).hint({"": idxName }).sort({a:1}).skip(10).limit(20).count();
   
   return rc ;
}

function checkResult( rc )
{
   println("\n---Begin to check result.");
   
   var expCnt = 40;
   var actCnt = Number( rc );
   if( expCnt !== actCnt )
   {
      throw buildException("checkResult", null, "[compare the records]", 
                          "[count:"+ expCnt +"]",
                          "[count:"+ actCnt +"]");
   }
}