/************************************
 * *@Description: 节点重启后，dropIndex检查统计信息 
 * *@author:      luweikang
 * *@createdate:  2019.06.01
 * *@testlinkCase:seqDB-18385
 * **************************************/

var csName = "cs18385";
var groupName = "group18385";

main();

function main()
{
	//判断独立模式
	if( true == commIsStandalone( db ) )
	{
	   println( "run mode is standalone" );
	   return;
	}
   
   var clName = "cl18385";
   var indexName = "index18385";
   var hostName = getHostName();
   var svcName = Number(RSRVPORTBEGIN);
   var dataPath = RSRVNODEDIR + svcName ;
   
   try
   {
      println("---create rg and node---");
      var rg = db.createRG(groupName);
      rg.createNode(hostName, svcName, dataPath);
      rg.start();
      
      println("---create cs and cl, insert record and create index---");
      var cl = db.createCS(csName).createCL(clName, {"Group": groupName});
      cl.createIndex(indexName, {"a": 1});
      for(var i = 0; i< 1000; i++)
      {
         cl.insert({a: i});
      }
      
      println("---analyze---");
      db.analyze();  

      var data = new Sdb(hostName, svcName);
      var cur = data.getCS("SYSSTAT").getCL("SYSINDEXSTAT").find({"Index": indexName});
      if(!cur.next())
      {
         throw buildException( "", "", "create index and analyze, data node shuold be had index info", "exist", "not exist" );
      }
      
      println("---restart group---");
      rg.stop();
      rg.start();
      
      println("---drop index---");
      db.getCS(csName).getCL(clName).dropIndex(indexName);
      
      println("---check data node SYSSTAT.SYSINDEXSTAT---");
      data = new Sdb(hostName, svcName);
      cur = data.getCS("SYSSTAT").getCL("SYSINDEXSTAT").find({"Index": indexName});
      if(cur.next())
      {
         throw buildException( "", "", "drop index, data node shuold be no index info", "not exist", "exist" );
      }
   }
   finally
   {
      try
      {
         db.dropCS( csName );
      }
      catch( e ){}
      db.removeRG(groupName);
   }
}

function getHostName()
{
   var groupArray = commGetGroups(db);
   var hostName = groupArray[0][1].HostName;
   return hostName;
}
