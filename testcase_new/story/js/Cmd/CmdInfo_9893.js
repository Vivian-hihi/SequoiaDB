/******************************************************************************
*@Description : test js object cmd function: getInfo
*               TestLink : 9893 获取Cmd对象信息
*@author      : Liang XueWang 
******************************************************************************/

// 测试获取cmd对象信息
CmdTest.prototype.testGetInfo = function()
{
   this.init() ;
   
   var info = this.cmd.getInfo().toObj() ;
   if( this.isLocal )  // 测试本地cmd对象信息
   {
      if( info.type != "Cmd" || info.isRemote != false )
      {
         throw buildException( "testGetInfo", null, "get local cmd info",
                               this, JSON.stringify( info ) ) ;
      }
   }
   else                // 测试远程cmd对象信息
   {
      if( info.type != "Cmd" || info.hostname != this.hostname ||
          info.svcname != this.svcname || info.isRemote != true )
      {
         throw buildException( "testGetInfo", 0, "get remote cmd info",
                               this, JSON.stringify( info ) ) ;
      }
   }
   
   this.release() ;   
}

function main()
{
   // 获取本地和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var ct1 = new CmdTest( localhost, CMSVCNAME ) ;
   var ct2 = new CmdTest( remotehost, CMSVCNAME ) ;
   var cts = [ ct1, ct2 ] ;
   
   for( var i = 0;i < cts.length;i++ )
   {
      // 测试获取cmd对象信息
      cts[i].testGetInfo() ;
   }
}

main()