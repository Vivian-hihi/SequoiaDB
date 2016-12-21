/******************************************************************************
*@Description : test js object oma function: getOmaInstallFile getOmaInstallInfo
*               TestLink: 10626 Oma获取Oma安装信息文件路径
*                         10627 Oma获取Oma安装信息                      
*@author      : Liang XueWang
******************************************************************************/

// 测试获取Oma安装文件和安装信息
OmaTest.prototype.testOmaInstall = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   // 测试getOmaInstallFile
   var file = this.oma.getOmaInstallFile() ;
   if( file != "/etc/default/sequoiadb" )
   {
      throw buildException( "testOmaInstall", null, "get oma install file " + this, 
                            "/etc/default/sequoiadb", file ) ;
   }
   
   // 测试getOmaInstallInfo
   try
   {
      var InstallInfo = this.oma.getOmaInstallInfo().toObj() ;
      var InstallFileContent = cmd.run( "cat /etc/default/sequoiadb" ).split( "\n" ) ;
   }
   catch( e )
   {
      throw buildException( "testOmaInstall", e, "get oma install info " + this, 0, e ) ;
   }
   checkResult( InstallInfo, InstallFileContent, "getOmaInstallInfo" ) ;

   this.oma.close() ;
   remote.close() ;
}

function main()
{
   // 获取本地和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var ot1 = new OmaTest( localhost, CMSVCNAME ) ;
   var ot2 = new OmaTest( remotehost, CMSVCNAME ) ;
   
   var ots = [ ot1, ot2 ] ;
   for( var i = 0;i < ots.length;i++ )
   {
      // 测试获取Oma安装文件和安装信息
      ots[i].testOmaInstall() ;
   }
}

main()