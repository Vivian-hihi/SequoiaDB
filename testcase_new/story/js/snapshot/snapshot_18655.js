/******************************************************************************
*@Description : test snapshot SDB_SNAP_COLLECTIONS
*                seqDB-18655:固定集合中插入/pop/查询记录，集合快照信息验证
*@auhor       : 赵育
******************************************************************************/
var nodeNameMaster= [];
function main()
{
   if(commIsStandalone( db )){
      println("------Deploy is standalone");
      return;
   }
   
   var csName = COMMCSNAME + "_18655";
   var clName = COMMCLNAME + "_18655";
   
   commDropCS( db, csName);
   commCreateCS( db, csName, false, "", {Capped:true});
   
   var dataGroupNames = getDataGroupNames();
   var groupName = dataGroupNames[0];
   var dbcl = commCreateCLByOption(db, csName, clName, {Capped:true, Size:1024, Compressed:false, Group:groupName});
   var masterNode = db.getRG(groupName).getMaster();
   var hostName = masterNode.getHostName();
   var serviceName = masterNode.getServiceName();
   nodeNameMaster.push(hostName + ":" + serviceName);
   
   var doc=[];
   for(var i=0; i<1000; i++)
   {
      doc.push({a:i,b:i,c:i});
   }
   
   dbcl.insert(doc);
   var expStatistics = [{TotalDataRead:0,TotalIndexRead:0,TotalDataWrite:1000,TotalIndexWrite:0,TotalUpdate:0,TotalDelete:0,TotalInsert:1000,TotalSelect:0,TotalRead:0,TotalWrite:1000,TotalTbScan:0,TotalIxScan:0}];
   checkStatistics(csName + "." + clName, nodeNameMaster, expStatistics);
   
   var cursor = dbcl.find();
   while(cursor.next())
   {
   }
   cursor.close();
   var expStatistics = [{TotalDataRead:1000,TotalIndexRead:0,TotalDataWrite:1000,TotalIndexWrite:0,TotalUpdate:0,TotalDelete:0,TotalInsert:1000,TotalSelect:1000,TotalRead:1000,TotalWrite:1000,TotalTbScan:1,TotalIxScan:0}];
   checkStatistics(csName + "." + clName, nodeNameMaster, expStatistics);
   
   dbcl.pop({LogicalID:0,Direction:-1})
   var expStatistics = [{TotalDataRead:1000,TotalIndexRead:0,TotalDataWrite:1000,TotalIndexWrite:0,TotalUpdate:0,TotalDelete:0,TotalInsert:1000,TotalSelect:1000,TotalRead:1000,TotalWrite:1000,TotalTbScan:1,TotalIxScan:0}];
   checkStatistics(csName + "." + clName, nodeNameMaster, expStatistics);
   
   db.resetSnapshot({Type:"collections"});
   var expStatistics = [{TotalDataRead:0,TotalIndexRead:0,TotalDataWrite:0,TotalIndexWrite:0,TotalUpdate:0,TotalDelete:0,TotalInsert:0,TotalSelect:0,TotalRead:0,TotalWrite:0,TotalTbScan:0,TotalIxScan:0}];
   checkStatistics(csName + "." + clName, nodeNameMaster, expStatistics);
   
}
main();