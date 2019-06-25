/* *****************************************************************************
@discretion: setSessionAttr(),set preferedStrict = true, assign node select when the node stop.
@author：2019-6-20 luweikang  Init
***************************************************************************** */

main();
//TODO:1、会话设置preferedStrict建议在文本用例中补充描述
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
   //TODO:1、这里的检查结果为啥要判断是备节点，已有可能这个节点会变成主节点
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
         db.dropCS(csName);//TODO:2、清理组建议放在finally里面，放在这里如果try前面的失败则组会残留
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
   //TODO:3、节点已经启动，为啥dropCS还要循环判断
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