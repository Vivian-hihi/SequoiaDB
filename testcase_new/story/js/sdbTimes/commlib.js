/*******************************************************************************
*@Description : matches testcase common functions and varialb
*@Modify list :
*              2017-02-28 xiaoni huang
*******************************************************************************/
var cmd  = cmdInit();

function readyCL( clName )
{
   println("\n---Begin to create CL.");
	
   commDropCL( db, COMMCSNAME, clName, true, true,
               "Failed to drop CL in the pre-condition." );
   
   var cl = commCreateCL( db, COMMCSNAME, clName, -1, true, true, false,
                          "Failed to create CL." );                          
   return cl;
}

function cleanCL( clName )
{
   println("\n---Begin to drop CL.");
	
   commDropCL( db, COMMCSNAME, clName, false, false,
               "Failed to drop CL in the end-condition" );
}

function cmdInit()
{
   try
   {
      var cmd = new Cmd();
      return cmd;
   }
   catch( e )
   {
      println("Failed to init cmd.");
      throw e;
   }
}

function cmdRun( str )
{
   try
   {
      var rc = cmd.run( str ).split("\n")[0];
      return rc;
   }
   catch( e )
   {
      println("Failed to exec cmd run.");
      throw e;
   }
}