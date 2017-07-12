/************************************
*@Description: create cappedCL and drop cappedCL
*@author:      luweikang
*@createdate:  2017.7.6
*@testlinkCase:seqDB-11763,seqDB-11764
**************************************/

main();

function main()
{
   var csName = CHANGEDPREFIX + "_11763_CS";
   var clName = CHANGEDPREFIX + "_11763_CL";
   
   //clean environment before test
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   //create cappedCS
   println( "begin to create cappedCS" );
   var options = {Capped:true};
   commCreateCS( db, csName, false, "create cappedCS", options );
   
   //create cappedCL
   var optionObj = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:false};
   commCreateCLByOption( db, csName, clName, optionObj, false, false, "create cappedCL" );
   
   //check cappedCL
   if( true === commIsStandalone( db ) )
   {
      standaloneCheckCreateCL( csName, clName );
   }
   else
   {
      var nodeList = getNodeList( CATALOG_GROUPNAME );
      checkCappedCL( csName, clName, nodeList );
   }
   
   //drop cappedCL
   println("drop cappedCL")
   commDropCL( db, csName, clName, false, false, "drop cappedCL" );
   
   //check drop cappedCL result
   if( true === commIsStandalone( db ) )
   {
      standaloneCheckDropCL( csName, clName );
   }
   else
   {
      checkDropCL( csName, nodeList);
   }
   println( "---end to drop cappedCL---" );
   
   //clean cappedCS after test
   commDropCS( db, csName, true, "drop CS in the end" );
}

function standaloneCheckCreateCL( csName, clName )
{
   var cursor = db.snapshot( SDB_SNAP_COLLECTIONS, {'Name':csName + "." + clName} );
   var attribute   =  cursor.next().toObj().Details[0].Attribute;
   if( attribute !== "NoIDIndex | Capped" )
   {
      throw buildException( "check cappedCL", null, "check cappedCL", "NoIDIndex | Capped",  attribute);
   }
   cursor.close();
}

function standaloneCheckDropCL( csName, clName )
{
   try
   {
      db.getCS( csName ).getCL( clName );
      throw "ERR_DROP_CL";
   }
   catch( e )
   {
      if( e !== -23 )
      {
         throw buildException( "check drop cappedCS", null, "check drop cappedCS", "-23",  e);
      }
   }
}

function getNodeList( groupName )
{
   var nodeList =[];
   var groupInfo = db.getRG( groupName ).getDetail().current().toObj().Group;
   for( var i in groupInfo )
   {
      var nodeInfo = groupInfo[i].HostName + ":" + groupInfo[i].Service[0].Name;
      nodeList.push( nodeInfo );
   }
   //println( "---get nodelist of " + groupName + ": " + nodeList );
   
   return nodeList;
}

function checkCappedCL( csName, clName, nodeList )
{
	for( var i in nodeList )
   {
	   var catadb = new Sdb( nodeList[i] );
	   var cursor = catadb.SYSCAT.SYSCOLLECTIONS.find({ 'Name' : csName + "." + clName});
      var obj    = cursor.next().toObj();
	   var type   = obj.Type;
      var attributeDesc = obj.AttributeDesc;
      var max    = obj.Max;
      var size   = obj.Size;
	   if( type !== 1 )
	   {
		   throw buildException( "check cappedCL type", null, "check cappedCL type", "1",  type);
	   }
      if( attributeDesc !== "NoIDIndex | Capped" )
	   {
		   throw buildException( "check cappedCL attributeDesc", null, "check cappedCL attributeDesc", "NoIDIndex | Capped",  attributeDesc);
	   }
      if( max == undefined )
	   {
		   throw buildException( "check cappedCL", null, "check cappedCL max", "not undefined",  max);
	   }
      if( size == undefined )
	   {
		   throw buildException( "check cappedCL", null, "check cappedCL size", "not undefined",  size);
	   }
	   cursor.close();
	   catadb.close();
   }
}

function checkDropCL( csName, clName, nodeList )
{
	for( var i in nodeList )
   {
	   var catadb = new Sdb( nodeList[i] );
      var count  = catadb.SYSCAT.SYSCOLLECTIONS.count({ 'Name' : csName + "." + clName});
	   if( count != 0 )
	   {
		   throw buildException( "check drop cappedCL", null, "check drop cappedCL", "0",  count );
	   }
	   catadb.close();
   }
}


