/******************************************************************************
*@Description : Public function for testing split.
*@Modify list :
*               2014-6-17  xiaoni Zhao  Init
******************************************************************************/

//get group name and service name
function getGroupNames()
{
   var groups = db.listReplicaGroups();
   var groupNames = new Array();
   while( groups.next() )
      if( groups.current().toObj()["GroupID"] >= 1000 )
      {
         groupNames.push( groups.current().toObj()["GroupName"] );
      }
   return groupNames;	
}

//get srcGroup
function getSrcGroup( clName )
{
	var clFullName = COMMCSNAME + "." + clName;
	var clInfo = db.snapshot( 8,{Name:clFullName} );
	while( clInfo.next() )
	{
		var clInfoObj = clInfo.current().toObj();
		var srcGroupName = clInfoObj.CataInfo[0].GroupName;
	} 
	return srcGroupName;
}

function checkIdIndex( clName, AttributeDesc, csName )
{
	if( csName === undefined )
	{
	   csName = COMMCSNAME;	
	}
	var clFullName = csName + "." + clName;
   var catalogInfo = db.snapshot( 8, {Name:clFullName} ).current().toObj();
   if( catalogInfo.AttributeDesc !== AttributeDesc )
   {
      throw "IDINDEX_ERROR";	
   }
}

function getDesGroup( groupNames, srcGroup )
{
   for( var i=0; i<groupNames.length; i++)
   {
   	if( groupNames[i] !== srcGroup )
   	{
         var desGroup = groupNames[i];
   		break;
   	}
   }	
   return desGroup;
}

function checkCataInfo( clName, srcGroup, cataInfoLen, csName )
{
	if( csName === undefined )
	{
	   csName = COMMCSNAME;	
	}
   var clFullName = csName + "." + clName;
   var catalogInfo = db.snapshot( 8,{Name:clFullName} ).current().toObj();
   if( catalogInfo.CataInfo.length !== cataInfoLen )
   {
      throw "SPLIT_ERROR";
   }	
}

function checkData( expRecs, clName )
{
   var actRecs = db.getCS( COMMCSNAME ).getCL( clName ).find().toArray();
   
   //check count
   if( expRecs.length != actRecs.length )
   {
      throw "COUNT_ERROR";	
   } 	
   
   //check records
   for( var i in expRecs )
	{
	   var actRec = actRecs[i];
	   var expRec = expRecs[i];	
	   for( var j in expRec)
	   {
	      if( JSON.stringify(actRec[j]) !== JSON.stringify(expRec[j]) )
	      {
	         println( "error occurs in " + (parseInt(i)+1) + "th record,in field '" + j + "';" );	
	         println("actual record =" + JSON.stringify(actRec) + "\nexpect record =" + JSON.stringify(expRec));
	         throw "RECORDS_ERROR";
	      }	
	   } 
   } 
}

function checkSplitResult( srcGroup, desGroup, clName, csName )
{
	if( csName === undefined )
	{
	   csName = COMMCSNAME;	
	}
   var clFullName = csName + "." + clName;
	var srcGroupDb = db.getRG( srcGroup ).getMaster().connect();
	var srcGroupCount = eval( "srcGroupDb." + clFullName + ".count()" );
	
	var desGroupDb = db.getRG( desGroup ).getMaster().connect();
	var desGroupCount = eval( "desGroupDb." + clFullName + ".count()" );
	
	if(srcGroupCount ===0 || desGroupCount == 0 || srcGroupCount + desGroupCount !==50 )
	{
	   throw "SPLIT_ERROR";	
	}	
}


function dropDomain( domName, csName )
{
	//drop CS before
   commDropCS( db, csName );
   
   try
	{
	   db.dropDomain( domName );
	}catch( e )
	{
		if( e !== -214 )
		{
			println( "drop domain failed!" ) ;
         throw e ;
		}	
	} 
}