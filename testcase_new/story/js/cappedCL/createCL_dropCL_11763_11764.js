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
   var optionObj = {Capped:true, Size:1024, Max:10000000, AutoIndexId:false};
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
   println( "---get nodelist of " + groupName + ": " + nodeList );
   
   return nodeList;
}

function checkCappedCL( csName, clName, nodeList )
{
	var repeatTime = 10;
	for(var i = 0; i < repeatTime; i++){
		  var j = i % nodeList.length;
		  println("j: " + j);
		  var catadb = new Sdb( nodeList[j] );
	      var cursor = catadb.SYSCAT.SYSCOLLECTIONS.find({ 'Name' : csName + "." + clName});
          if(cursor == null){
			  // wait for the slave node sync
			  sleep(5 * 60 * 1000);//5 mins		
              cursor = catadb.SYSCAT.SYSCOLLECTIONS.find({ 'Name' : csName + "." + clName});			  
		  }
          
		  if(cursor != null){
			   var obj    = cursor.next().toObj();
               var attributeDesc = obj.AttributeDesc;
               var max    = obj.Max;
               var size   = obj.Size;
			   if( attributeDesc !== "NoIDIndex | Capped" || max == undefined 
			      || size == undefined)
	           {
		          throw buildException( "check cappedCL attributeDesc", null, "check cappedCL attributeDesc", "NoIDIndex | Capped",  attributeDesc);
	           }
	          cursor.close();
	          catadb.close();  
		  }else{
			  throw buildException( "check cappedCL failed , cursor is null");
		  }
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


