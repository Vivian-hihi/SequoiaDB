/******************************************************************************
*@Description : seqDB-19589:检查list(SDB_LIST_SVCTASKS)列表信息
*               TestLink : seqDB-19589
*@author      : wangkexin
*@Date        : 2019-09-27
******************************************************************************/
main(db) ;
function main()
{
   var groups = commGetGroups( db, false, "", true, true, false );
   var dataNodeNames = [];
   for(var i=0 ;i<groups.length;i++)
   {
      dataNodeNames.push(groups[i][1].HostName + ":" + groups[i][1].svcname);
   }
   
   for(var index in dataNodeNames)
   {
      var cur = db.list(SDB_LIST_SVCTASKS , {"NodeName": dataNodeNames[index],"TaskID": 0,"TaskName": "Default"});
      var arr = cur.toArray();
      if(arr.length != 1)
      {
         println("act arr :" + arr);
         throw buildException("check list failed", "", "list(SDB_LIST_SVCTASKS, {'NodeName': " + dataNodeNames[index] + ",'TaskID': 0,'TaskName': 'Default'}", "returned number is 1", "returned number is " + arr.length);
      }
   }
}