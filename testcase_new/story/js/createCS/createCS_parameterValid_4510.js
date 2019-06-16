/****************************************************
@description: seqDB-4510:createCS，覆盖options：PageSize有效字符和边界
@author:
              2019-6-4 wuyan init
****************************************************/
//main();
function main()
{   
   var csName = CHANGEDPREFIX + "cs4510";   
   var pageSizes = [ 0, 4096, 8192, 16384, 32768, 65536 ];
   
   for ( var i = 0 ; i < pageSizes.length; i++ )
   {
      var pageSize = pageSizes[i];     
      //TODO :clear cs in the beginning.
      commDropCS( db, csName, true, "clear cs in the beging." );
      createCSAndCheckResult( csName, pageSize );
      commDropCS( db, csName, false, "clear cs in the ending." );      
   }   
}

function createCSAndCheckResult( csName, pageSize )
{
   println("\n---Begin to createCS. pageSize =" + pageSize );   
   
   var options = { PageSize : pageSize };
   //create cs;
   var dbcs = db.createCS( csName, options );
   
   //create cl in the cs
   var clName = "cl4510";
   dbcs.createCL( clName );
   
   //check the options
   var cursor = db.snapshot( 5, { Name : csName});
   var actPageSize = 0;
   while (cursor.next())
   {
      var curInfo = cursor.current();
      actPageSize = curInfo.toObj().PageSize;
   }
   
   var expPageSize = pageSize;
   if( pageSize == 0 )
   {
      expPageSize = 65536;      
   }
   
   if ( Number(expPageSize) !== Number(actPageSize) )
   {
      throw buildException("check pageSize","","","pageSize:" + expPageSize,"actPageSize:" + actPageSize);
   }   
   
   
}

