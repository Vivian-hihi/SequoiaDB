/*******************************************************************************
*@Description :  common function
*@Modify list :
*                2016/7/14  XiaoNi Huang Init
*******************************************************************************/

/* ****************************************************
@description: create cl
@return: cl
**************************************************** */
function readyCL( csName, clName, optionObj, message )
{
   if( message == undefined ) { message = ""; }
   println("\n---Begin to create CL "+ message +".");
   
   if( optionObj == undefined ) { optionObj = {ReplSize:0}; }
	
   commDropCL( db, csName, clName, true, true, 
                      "Failed to drop CL in the pre-condition." );
   
   var cl = commCreateCLByOption( db, csName, clName, optionObj, true, true, 
                                         "Failed to create CL." )
   return cl;
}

/* ****************************************************
@description: drop cl
**************************************************** */
function cleanCL( csName, clName )
{
   println("\n---Begin to drop CL.");
	
   commDropCL( db, csName, clName, false, false,
                      "Failed to drop CL in the end-condition" );
}

/* ****************************************************
@description: insert data
**************************************************** */

function readyData( cl )
{
	try
	{
		println("\n---Begin to insert cl data.");
		cl.insert({_id:1,a:1,b:"b1"});
		cl.insert({_id:2,a:3,b:"b2"});
		cl.insert({_id:3,a:2,b:"b3"});
		cl.insert({_id:4,a:[10,20,30],b:"b4"});
	}
	catch(e)
	{
		throw buildException("insertData()",e,"insert", "insert success","insert fail");
	}
}

/* ****************************************************
@description: check cl data
**************************************************** */
function checkCLData( rc, expRecs, expCnt )
{
   println("\n---Begin to check cl data.");
   var recsArray = [];
   while( tmpRecs = rc.next() )
   {
	   recsArray.push( tmpRecs.toObj() );
   }
   var expCnt = 4;
   var actCnt  = recsArray.length;
   var actRecs = JSON.stringify( recsArray );
   if( actCnt !== expCnt || actRecs !== expRecs )
   {
	   throw buildException( "checkCLdata", null, "[find]",
		  "[cnt:"+ expCnt +", recs:"+ expRecs +"]",
		  "[cnt:"+ actCnt +", recs:"+ actRecs +"]" );
   }
   println( "cl records: "+ actRecs );
}