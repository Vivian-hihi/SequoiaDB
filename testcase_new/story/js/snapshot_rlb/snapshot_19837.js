/******************************************************************************
*@Description : seqDB-19837 : 指定showError和showErrorMode查询数据库快照 
*@author      : Zhao xiaoni
*@Date        : 2019-10-15
******************************************************************************/
import("../snapshot/commlib.js");
try
{
   main() ;
}
catch(e)
{
   if(e.constructor === Error)
   {
      println(e.stack);
   }
   throw e;
}

function main()
{
   if( commIsStandalone( db ) )
   {
      println("The environment is standalone!");
      return;
   }
   
   var nodeAddresses = getNodeAddresses();
   stopNodes( nodeAddresses );
   
   try
   {
      //show/[aggr, flat]显示节点错误信息
      var showError = "show";
      var showErrorMode = ["aggr", "flat"];
      for(var i = 0; i < showErrorMode.length; i++)
      {
         var count = 0;
         var sdbsnapshotOption = new SdbSnapshotOption().options({ShowError: showError, ShowErrorMode: showErrorMode[i]});
         var cursor = db.snapshot(SDB_SNAP_DATABASE, sdbsnapshotOption);
         var errNodes = cursor.current().toObj()["ErrNodes"];
         for(var i = 0; i < nodeAddresses.length; i++)
         {
            var hostName = nodeAddresses[i]["hostName"];
            var svcName = nodeAddresses[i]["svcName"];
            for(var i = 0; i < errNodes.length; i++)
            {
               var nodeName = errNodes[i]["NodeName"];
               var flag = errNodes[i]["Flag"];
               if(nodeName === hostName+":"+svcName)
               {
                  if(flag !== -79)
                  {
                     throw new Error("show/aggr's nodeName is " + nodeName);
                  }
                  count++;
                  break;
               }
            } 
         }   
         if(count !== nodeAddresses.length)
         {
            throw new Error("show/aggr's count is " + count);
         }
      }
   
      //ignore/[flat, aggr]显示节点错误信息
      showError = "ignore";
      showErrorMode = ["flat", "aggr"];
      for(var i = 0; i < showErrorMode.length; i++)
      {
         sdbsnapshotOption = new SdbSnapshotOption().options({ShowError: showError, ShowErrorMode: showErrorMode[i]});
         cursor = db.snapshot(SDB_SNAP_HEALTH, sdbsnapshotOption);
         errNodes = cursor.current().toObj()["ErrNodes"];
         if(errNodes !== undefined)
         {
            throw new Error("ignore/aggr's errNodes is " + JSON.stringify(errNodes));
         }
      }
   
      //only/[aggr, flat]显示节点错误信息
      showError = "only";
      showErrorMode = ["aggr", "flat"];
      for(var i = 0; i < showErrorMode.length; i++)
      {
         count = 0;
         var num = 0;
         sdbsnapshotOption = new SdbSnapshotOption().options({ShowError: showError, ShowErrorMode: showErrorMode[i]});
         cursor = db.snapshot(SDB_SNAP_HEALTH, sdbsnapshotOption);
         while(cursor.next())
         {
            num++;
            errNodes = cursor.current().toObj()["ErrNodes"];
            for(var i = 0; i < nodeAddresses.length; i++)
            {
               var hostName = nodeAddresses[i]["hostName"];
               var svcName = nodeAddresses[i]["svcName"];
               for(var i = 0; i < errNodes.length; i++)
               {
                  var nodeName = errNodes[i]["NodeName"];
                  var flag = errNodes[i]["Flag"];
                  if(nodeName === hostName+":"+svcName)
                  {
                     if(flag !== -79)
                     {
                        throw new Error("show/aggr's nodeName is " + nodeName);
                     }
                     count++;
                     break;
                  }
               } 
            }
         }
         if(num !== 1 || count !== nodeAddresses.length)
         {
            throw new Error("only/aggr's num is " + num +", count is " + count);
         }
      }
   }
   finally
   {
      startNodes( nodeAddresses );
   }
}
