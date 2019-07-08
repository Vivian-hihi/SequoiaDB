
function checkStatistics(clFullName, nodeNameMaster, expStatistics)
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

function getStatistics(clFullName, nodeNameMaster)
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
            tmpObj.TotalIndexRead = group[i].TotalIndexRead;
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

function getDataGroupNames()
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