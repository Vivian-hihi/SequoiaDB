/****************************************************
@description:	commlib of FullText		
@Date : 2018-09-18
@Author: liuxiaoxuan
                     
****************************************************/
import ( "../lib/fulltext_commlib.js" );

/*****************************************************************
*@Description: 检查数据组的所有节点是否正常，且主备LSN是否一致
@input:        timeoutSecond
               csName
               clName
******************************************************************/
function checkGroupBusiness( timeoutSecond, csName, clName )
{ 
   var groupNames = commGetCLGroups( db, csName + "." + clName );
   for( var i in groupNames )
   {
      var doTimes = 1;
      while( doTimes <= timeoutSecond )
      {      
         if( !isSuccesscreateTestCollection( groupNames[i] ) && !isNodesNormal( groupNames[i] ) )
         {
            doTimes++;
            sleep( 1000 );
         }
         else 
         {
            break;
         }
      } 
   
      if( doTimes > timeoutSecond )
      {
         throw buildException( "checkGroupBusiness()", null, "check group bussiness", "success", "time out" );
      }
   
      checkConsistency( csName, clName );
      println( "check group bussiness success!" );
   }
}

/*****************************************************************
*@Description: 指定数据组、强一致性创建集合
@input:        groupName
******************************************************************/
function isSuccesscreateTestCollection( groupName )
{
    var clName = "clForTestBusiness_reliability_js";
    try
    {
        commDropCL( db, COMMCSNAME, clName, true, true );
        var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, { "ReplSize" : -1, "Group": groupName }, false, false );
        return true;
    }
    catch ( e )
    {
        //println( "isSuccesscreateTestCollection throw error: " + e );
        return false;
    }
    finally
    {
        commDropCL( db, COMMCSNAME, clName, true, true );
    }
}


/*****************************************************************
*@Description: 检查catalog的主备LSN是否一致
@input:        csName
               clName
******************************************************************/
function checkCatalogBusiness( timeoutSecond )
{
   // 1. 首先检查所有节点是否能正常连接
   var doTimes = 1;
   while( doTimes <= timeoutSecond )
   {      
      if( !isNodesNormal( "SYSCatalogGroup" ) )
      {
         doTimes++;
         sleep( 1000 );
      }
      else 
      {
         break;
      }
   } 
   
   if( doTimes > timeoutSecond )
   {
      throw buildException( "checkCataNodesNormal()", null, "check catalog nodes normal", "success", "time out" );
   }
   
   // 2. 校验主备节点LSN
   doTimes = 1;
   while( doTimes <= timeoutSecond )
   {      
      var primaryNodeLSNs = getPrimaryNodeLSNs( ["SYSCatalogGroup"] );
      if( !checkLSN( ["SYSCatalogGroup"] , primaryNodeLSNs) )
      {
         doTimes++;
         sleep( 1000 );
      }
      else 
      {
         println( "check catalog bussiness success!" );
         break;
      }
   } 
   
   if( doTimes > timeoutSecond )
   {
      throw buildException( "checkCatalogBusiness()", null, "check catalog business", "success", "time out" );
   }
}

/*****************************************************************
*@Description: 检查连接节点是否正常
@input:        groupName
******************************************************************/
function isNodesNormal( groupName )
{
    try
    {
       var rg = db.getRG( groupName );
       var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")" );
       var nodesInGroup = rgDetail.Group;
       for( var i = 0; i < nodesInGroup.length; ++i )
       {
          var hostName = nodesInGroup[i].HostName;
          var serviceName = nodesInGroup[i].Service[0].Name;
          new Sdb( hostName, serviceName );              
       }
       return true;
    }
    catch ( e )
    {
        if( -104 != e && -79 != e && -134 != e )
        {
            throw e;
        }
        return false;
    }      
}
