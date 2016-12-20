/******************************************************************************
*@Description : test get oma info function: getOmaInstallFile getOmaInstallInfo
*               getOmaConfigFile getOmaConfigs setOmaConfigs addAOmaSvcName 
*               delAOmaSvcName getAOmaSvcName
*               TestLink: 10626 10627 10628 10629 10630 10631
*                         10632 10633 10634 10635 
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
      throw buildException( "testOmaInstallFileAndInfo", 0, "get oma install file", "/etc/default/sequoiadb", file ) ;
   }
   
   // 测试getOmaInstallInfo
   try
   {
      var InstallInfo = this.oma.getOmaInstallInfo().toObj() ;
      var InstallFileContent = cmd.run( "cat /etc/default/sequoiadb" ).split( "\n" ) ;
   }
   catch( e )
   {
      throw buildException( "testOmaInstallFileAndInfo", e, "get oma install info", 0, e ) ;
   }
   checkResult( InstallInfo, InstallFileContent, "getOmaInstallInfo" ) ;

   this.oma.close() ;
   remote.close() ;
}

// 测试获取cm配置文件和配置信息，测试设置cm配置信息
OmaTest.prototype.testOmaConfig = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   // 测试getOmaConfigFile
   var ConfigFile = this.oma.getOmaConfigFile() ;
   var InstallPath = commGetInstallPath() ;
   var ExpectFile = InstallPath + "/conf/sdbcm.conf" ;
   if( ConfigFile != ExpectFile )
   {
      throw buildException( "testOmaConfig", 0, "get oma config file", ExpectFile, ConfigFile ) ;
   }
   
   // 测试getOmaConfigs
   try
   {   
      var Configs = this.oma.getOmaConfigs().toObj() ;
      var ConfigFileContent = cmd.run( "cat " + ExpectFile ).split( "\n" ) ;
   }
   catch( e )
   {
      throw buildException( "testOmaConfig", e, "get oma configs", 0, e ) ;
   }
   checkResult( Configs, ConfigFileContent, "getOmaConfigs" ) ;
   
   // 测试getOmaConfigs时，文件不存在
   try
   {
      this.oma.getOmaConfigs( "/opt/notexist" ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -4 )
      {
         throw buildException( "getOmaConfigs", e, "get oma configs with not exist file", -4, e ) ;
      }  
   }
   
   // 测试getOmaConfigs时，文件不是oma配置文件
   try
   {
      this.oma.getOmaConfigs( InstallPath + "/bin/sdb" ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -6 )
      {
         throw buildException( "getOmaConfigs", e, "get oma configs with illegal file", -6, e ) ;
      }
   }
      
   // 测试setOmaConfigs
   Configs[ "name" ] = "lxw" ;
   this.oma.setOmaConfigs( Configs ) ;
   var Actual = this.oma.getOmaConfigs().toObj().name ;
   if( Actual != "lxw" )
   {
      throw buildException( "setOmaConfigs", e, "check set oma configs", "lxw", Actual ) ;
   }
   
   // 测试完成后删除新加的配置
   delete Configs[ "name" ] ;
   this.oma.setOmaConfigs( Configs ) ;

   this.oma.close() ;
   remote.close() ;  
}

// 测试增加、删除、获取oma端口
OmaTest.prototype.testOmaSvcName = function()
{
   this.testInit() ;
   
   // 测试addAOmaSvcName   
   this.oma.addAOmaSvcName( "test", "19000" ) ;
   var result = this.oma.getAOmaSvcName( "test" ) ;
   if( result != "19000" )
   {
      throw buildException( "testOmaSvcName", 0, "add a oma svcname", "19000", result ) ;
   }
   // 测试addAOmaSvcName,isReplace为true
   try
   {
      this.oma.addAOmaSvcName( "test", "18900", true ) ;
   }
   catch( e )
   {
      throw buildException( "testOmaSvcName", e, "add a exist oma svcname when isReplace is true", 0, e ) ;
   }
   // 测试addAOmaSvcName,isReplace为false
   try
   {
      this.oma.addAOmaSvcName( "test", "19000", false ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -6 )
      {
         throw buildException( "testOmaSvcName", e, "add a exist oma svcname when isReplace is false", -6, e ) ;   
      }
   }
   
   // 测试delAOmaSvcName  
   this.oma.delAOmaSvcName( "test" ) ;
   var omaSvcName = this.oma.getAOmaSvcName( "test" ) ;
   if( omaSvcName != "11790" )
   {
      throw buildException( "testOmaSvcName", e, "get a oma svcname after del", -6, e ) ;
   }

   this.oma.close() ;
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
      try
      {
         // 测试获取Oma安装文件和安装信息
         ots[i].testOmaInstall() ;
         
         // 测试获取Oma配置文件和配置信息，设置配置信息
         ots[i].testOmaConfig() ;
         
         // 测试增加、删除、获取Oma端口
         ots[i].testOmaSvcName() ;
      }
      catch( e )
      {
         ots[i].toString() ;
         throw e ;
      }
   }
}

main()