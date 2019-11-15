function checkStatistics(clFullName, nodeNameMaster, expStatistics)
{
   try
   {
      var actStatistics = getStatistics(clFullName, nodeNameMaster);
      for(var i=0; i<expStatistics.length; i++)
      {
         if(JSON.stringify(actStatistics).indexOf(JSON.stringify(expStatistics[i])) == -1)
         {
            println("actStatistics:" + JSON.stringify(actStatistics));
            println("expStatistics:" + JSON.stringify(expStatistics));
            throw "ERR";
         }
      }
   }
   catch(e)
   {
      throw new Error(e);
   }
}

function getStatistics(clFullName, nodeNameMaster)
{
   try
   {
      var cursor = db.snapshot(SDB_SNAP_COLLECTIONS, {Name: clFullName});
      var tmpArray = [];
      while(cursor.next())
      {
         var details = cursor.current().toObj().Details;
         for(var k=0; k< details.length; k++)
         {
            var group = cursor.current().toObj().Details[k].Group;
            for(var i=0; i<group.length; i++)
            {
               var tmpObj = {};
               tmpObj.NodeName = group[i].NodeName;
               tmpObj.TotalDataRead = group[i].TotalDataRead;
               //事务transuserbs=true时，可能产生额外TotalIndexRead，不对该值进行比较
               //tmpObj.TotalIndexRead = group[i].TotalIndexRead;
               tmpObj.TotalDataWrite = group[i].TotalDataWrite;
               tmpObj.TotalIndexWrite = group[i].TotalIndexWrite;
               tmpObj.TotalUpdate = group[i].TotalUpdate;
               tmpObj.TotalDelete = group[i].TotalDelete;
               tmpObj.TotalInsert = group[i].TotalInsert;
               tmpObj.TotalSelect = group[i].TotalSelect;
               tmpObj.TotalRead = group[i].TotalRead;
               tmpObj.TotalWrite = group[i].TotalWrite;
               tmpObj.TotalTbScan = group[i].TotalTbScan;
               tmpObj.TotalIxScan = group[i].TotalIxScan;
               tmpArray.push(tmpObj);
            }
         } 
      }  
      var statArray = [];
      for(var i=0; i<tmpArray.length; i++)
      {
         for(var j=0; j<nodeNameMaster.length; j++)
         {
            var actObj = {};
            if(tmpArray[i].NodeName == nodeNameMaster[j])
            {
               delete tmpArray[i].NodeName;
               actObj = tmpArray[i]; 
               statArray.push(actObj);
            }
         }
      }
      return statArray;
   }
   catch(e)
   {
      throw new Error(e);
   }
}

function getDataGroupNames()
{
   try
   {
      var groups = commGetGroups( db, false, "", false, false, false );
      var dataGroupNames = [];
      for(var i=0 ;i<groups.length;i++)
      {
         var tmpArray = groups[i];
         if(tmpArray[0].GroupName !== "SYSCatalogGroup" && tmpArray[0].GroupName !== "SYSCoord")
         {
            dataGroupNames.push(tmpArray[0].GroupName);
         }
      }   
      return dataGroupNames; 
   }
   catch(e)
   {
      throw new Error(e);
   }
}

function checkResult(actResult, expResult)
{
   try
   {
      if(actResult.length !== expResult.length)
      {
         throw "expectResult.length is " + expResult.length + ", but actResult.length is " + actResult.length;
      }
      if(JSON.stringify(actResult) !== JSON.stringify(expResult))
      {
         throw "expectResult is " + JSON.stringify(expResult) + ", but actResult is " + JSON.stringify(actResult);
      }
   }
   catch(e)
   {
      throw new Error(e);
   }
}


/************************************
*@Description: get a coord nod, a cata node, a data node
*@author:      zhaoxiaoni
*@createDate:  2019/10/21
**************************************/
function getNodeAddresses()
{
   try 
   {
      var cata = true;
      var data = true;
      var coord = true;
      var nodeAddresses = new Array();
      var cursor = db.listReplicaGroups();
   
      while(cursor.next())
      {
         var groupObj = cursor.current().toObj();
         var groupArray = groupObj["Group"];
         for(var i = 0; i < groupArray.length; i++)
         {
            var hostName = groupArray[i]["HostName"];
            var svcName = groupArray[i]["Service"][0]["Name"];
            var json = {"hostName": hostName, "svcName": svcName};
            var remote = new Remote(COORDHOSTNAME, 11790);
            var cmd = remote.getCmd();
            var remoteHostName = cmd.run("hostname").split("\n")[0];
            if(groupObj["Role"] === 1 && hostName != remoteHostName && coord == true)
            {
               nodeAddresses.push(json);
               coord = false;
               println("coordNode is " + hostName + ":" + svcName);
            }
            else if(groupObj["Role"] === 2 && cata == true)
            {
               nodeAddresses.push(json);
               cata = false;
               println("cataNode is " + hostName + ":" + svcName);
            }
            else if(groupObj["Role"] === 0 && data == true)
            {
               nodeAddresses.push(json);
               data = false;
               println("dataNode is " + hostName + ":" + svcName);
            }
         }
      }
      return nodeAddresses;
   }
   catch(e)
   {
      throw new Error(e);
   }
}

function stopNodes( nodeAddresses )
{
   try
   {
      var installDir = commGetInstallPath();
      command = installDir + "/bin/sdbstop -p ";
   
      for(var i=0; i<nodeAddresses.length; i++)
      {
         var hostName = nodeAddresses[i]["hostName"];
         var svcName = nodeAddresses[i]["svcName"];
         var remote = new Remote(hostName, 11790);
         var cmd = remote.getCmd();
         cmd.run(command + svcName);
      } 
   }
   catch(e)
   {
      throw new Error(e);
   }
}

function startNodes( nodeAddresses )
{
   try
   {
      var installDir = commGetInstallPath();
      command = installDir + "/bin/sdbstart -p ";
      
      for(var i=0; i<nodeAddresses.length; i++)
      {
         var hostName = nodeAddresses[i]["hostName"];
         var svcName = nodeAddresses[i]["svcName"];
         var remote = new Remote(hostName, 11790);
         var cmd = remote.getCmd();
         cmd.run(command + svcName);
      }
   }
   catch(e)
   {
      throw new Error(e);
   }
}
