/* *****************************************************************************
@discretion: alter cs,the test scenario is as follows:             
             test 15038: alter pagesize
             test 15040: alter lobPageSize
@author£º2018-4-27 wuyan  Init
***************************************************************************** */
var csName = CHANGEDPREFIX + "_cs15038"; 

main(db);
function main(db)
{	  
	try
	{
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      }    
	   //clean environment before test
	   commDropCS( db, csName, true, "drop cs" );         
  
      //create cs           
      var dbcs = commCreateCS( db, csName, false, "create CS");           
      
      //testcase:15038/15040
      println("---alter CS:pageSize and LobPageSize,the cs is no exist cl");
      var pageSize    = 4096;      
      var lobPageSize = 524288;      
      dbcs.setAttributes({PageSize:pageSize, LobPageSize:lobPageSize});
      checkAlterCSResult(csName, "PageSize", pageSize);
      checkAlterCSResult(csName, "LobPageSize", lobPageSize);   
      
      //clean
      //commDropCS( db, csName, true, "clear cs" );       
   }
   catch( e )
   {
      throw e; 
   }
   finally
   {
      if( db != null )
      {
         db.close()
      }
   }   
}





