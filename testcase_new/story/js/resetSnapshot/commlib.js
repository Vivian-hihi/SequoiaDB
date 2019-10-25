function checkResult( db, fullName, expResult )
{
   println("start to check result");
   var actResult = {};
   var cursor = db.snapshot(SDB_SNAP_COLLECTIONS, {Name: fullName});
   var groupArray = cursor.current().toObj()["Details"][0]["Group"];
   for(var i = 0; i < groupArray.length; i++)
   {
      actResult.TotalDataRead = groupArray[i]["TotalDataRead"];
      actResult.TotalIndexRead = groupArray[i]["TotalIndexRead"];
      actResult.TotalDataWrite = groupArray[i]["TotalDataWrite"];
      actResult.TotalIndexWrite = groupArray[i]["TotalIndexWrite"];
      actResult.TotalUpdate = groupArray[i]["TotalUpdate"];
      actResult.TotalDelete = groupArray[i]["TotalDelete"];
      actResult.TotalInsert = groupArray[i]["TotalInsert"];
      actResult.TotalSelect = groupArray[i]["TotalSelect"];
      actResult.TotalRead = groupArray[i]["TotalRead"];
      actResult.TotalWrite = groupArray[i]["TotalWrite"];
      actResult.TotalTbScan = groupArray[i]["TotalTbScan"];
      actResult.TotalIxScan = groupArray[i]["TotalIxScan"];
      
      if(JSON.stringify(actResult) !== JSON.stringify(expResult))
      {
         throw new Error("actResult: " + JSON.stringify(actResult) + "!== expResult: " + JSON.stringify(expResult));
      }
   }
}
