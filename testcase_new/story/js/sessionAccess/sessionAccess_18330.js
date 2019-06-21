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
   var groupName = "group18330";
   var instanceidList = [ 9, 8, 10 ];
   var nodeNum = 3;
   var nodeList = createRGAndNode(db, groupName, instanceidList, nodeNum);
   var expSvcNameList = getSvcNameList(db, groupName);
   
   //create cl ,then insert data 
   var csName = CHANGEDPREFIX + "_cs18330";
   var clName = CHANGEDPREFIX + "_cl18330";
   var dbcl = commCreateCLByOption( db, csName, clName, {ReplSize:0,Group:groupName});  
   insertData( dbcl);
   
   println("---begin to set and query instanceid is " + instanceidList[1]);
   db.setSessionAttr( { PreferedInstance: instanceidList[1], PreferedStrict: true } );
   setSessionAttrAndCheckResult(db, dbcl, groupName, expSvcNameList[1], false );

   //stop node and select record
   var rg = db.getRG(groupName);
   try
   {
      println("begin to stop node: " + expSvcNameList[1]);
      rg.getNode(nodeList[1].hostname, nodeList[1].svcname).stop();
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
   println("restart the node: " + expSvcNameList[1]);
   rg.getNode(nodeList[1].hostname, nodeList[1].svcname).start();
   db.setSessionAttr({PreferedStrict: true});
   setSessionAttrAndCheckResult(db, dbcl, groupName, expSvcNameList[1], false );
   
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