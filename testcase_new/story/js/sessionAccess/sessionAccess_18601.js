/* *****************************************************************************
@discretion: setSessionAttr(),set preferedStrict = true, assign node select when the node stop.
@author：2019-6-20 luweikang  Init
***************************************************************************** */

main();

function main()
{
   var db = new Sdb(COORDHOSTNAME, COORDSVCNAME ) ; 
   if( true == commIsStandalone( db ) )
   {
      println( "run mode is standalone" );
      return;
   }  
   
   //create group and node
   var groupName = "group18601";
   var instanceidList = [ 9, 8, 10, 11 ];
   var nodeNum = 4;
   var nodeList = createRGAndNode(db, groupName, instanceidList, nodeNum);
   var expSvcNameList = getSvcNameList(db, groupName);
   
   //create cl ,then insert data 
   var csName = CHANGEDPREFIX + "_cs18601";
   var clName = CHANGEDPREFIX + "_cl18601";
   var dbcl = commCreateCLByOption( db, csName, clName, {ReplSize:0,Group:groupName});  
   insertData( dbcl);
   
   println("---begin to set and query instanceid is " + instanceidList[1] + " and " + instanceidList[2]);
   db.setSessionAttr( { PreferedInstance: [instanceidList[1], instanceidList[2]], PreferedStrict: true } );

   //stop node and select record
   println("begin to stop node: " + expSvcNameList[1]);
   var rg = db.getRG(groupName);
   rg.getNode(nodeList[1].hostname, nodeList[1].svcname).stop();
   setSessionAttrAndCheckResult(db, dbcl, groupName, expSvcNameList[2], false );
   
   try
   {
      println("begin to stop node: " + expSvcNameList[2]);
      rg.getNode(nodeList[2].hostname, nodeList[2].svcname).stop();
      dbcl.find().explain();
      throw "FIND_SHOULD_FAIL";
   }
   catch(e)
   {
      if(e != -250)
      {
         rg.start();
         sleep(1000);
         db.dropCS(csName);
         db.removeRG(groupName);
         throw e;
      }
   }
   db.setSessionAttr({PreferedStrict: false});
   dbcl.find().explain();
   
   //restart the node and select record
   println("restart the group: " + groupName);
   rg.start();
   db.setSessionAttr({PreferedStrict: true});
   dbcl.find().explain();
   
   var sleepInteval=10;
   for(var i = 0; i < 1000; i++)
   {
      try
      {
         db.dropCS(csName);
         break;
      }
      catch(e)
      {
         if(e != -105)
         {
            throw e;
         }
         sleep(sleepInteval);
      }
   }
   
   db.removeRG(groupName);
}

function setSessionAttrAndCheckResult(db,dbcl, groupName, expQueryNode, isPrimary )
{
   var queryNode = getAccessNode( dbcl);
   checkAcessNodeResult( queryNode, expQueryNode );
   checkAccessNodeIsPrimary( queryNode, groupName, isPrimary );
}