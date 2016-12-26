/******************************************************************************
*@Description : test js object oma function: getOmaConfigFile getOmaConfigs 
*                                            setOmaConfigs 
*               TestLink: 10628 Oma获取Oma配置文件路径
*                         10629 Oma获取Oma配置信息
*                         10630 Oma获取Oma配置信息，文件不存在
*                         10631 Oma获取Oma配置信息，文件不是oma配置文件
*                         10632 Oma设置Oma配置信息 
*@author      : Liang XueWang
******************************************************************************/

// 测试正常获取oma配置文件和配置信息
OmaTest.prototype.testGetOmaConfigsNormal = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   // 测试getOmaConfigFile
   var configFile = this.oma.getOmaConfigFile() ;
   var installPath = toolGetSequoiadbDir( this.hostname, this.svcname ) ;
   var expectFile = installPath + "/conf/sdbcm.conf" ;
   if( configFile != expectFile )
   {
      throw buildException( "testGetOmaConfigsNormal", null, "get oma config file " + this, 
                            expectFile, configFile ) ;
   }
   
   // 测试getOmaConfigs
   try
   {   
      var configs = this.oma.getOmaConfigs().toObj() ;
      var configFileContent = cmd.run( "cat " + expectFile ).split( "\n" ) ;
   }
   catch( e )
   {
      throw buildException( "testGetOmaConfigsNormal", e, "get oma configs " + this, 0, e ) ;
   }
   checkResult( configs, configFileContent, "getOmaConfigs" ) ;

   this.oma.close() ;
   remote.close() ;  
}

// 测试获取oma配置信息异常
OmaTest.prototype.testGetOmaConfigsAbnormal = function()
{
   this.testInit() ;
   var installPath = toolGetSequoiadbDir( this.hostname, this.svcname ) ;
   // 测试getOmaConfigs时，文件不存在
   try
   {
      this.oma.getOmaConfigs( "/opt/notexist" ) ;
      throw "get oma configs with not exist file should be failed" ;
   }
   catch( e )
   {
      if( e != -4 )
      {
         throw buildException( "testGetOmaConfigsAbnormal", e, 
               "get oma configs with not exist file " + this, -4, e ) ;
      }  
   }
   
   // 测试getOmaConfigs时，文件不是oma配置文件
   try
   {
      this.oma.getOmaConfigs( installPath + "/bin/sdb" ) ;
      throw "get oma configs with sdb file should be failed" ;
   }
   catch( e )
   {
      if( e != -6 )
      {
         throw buildException( "testGetOmaConfigsAbnormal", e, 
               "get oma configs with sdb file " + this, -6, e ) ;
      }
   }
   
   this.oma.close() ;
}

// 测试设置oma配置信息
OmaTest.prototype.testSetOmaConfigs = function()
{
   this.testInit() ;
   
   // 测试setOmaConfigs
   var configs = this.oma.getOmaConfigs().toObj() ;  
   configs[ "name" ] = "lxw" ;
   this.oma.setOmaConfigs( configs ) ;
   var name = this.oma.getOmaConfigs().toObj().name ;
   if( name != "lxw" )
   {
      throw buildException( "testSetOmaConfigs", null, 
            "check set oma configs " + this, "lxw", name ) ;
   }
   
   // 测试完成后删除新加的配置
   delete configs[ "name" ] ;
   this.oma.setOmaConfigs( configs ) ;
   
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
      // 测试正常获取Oma配置文件和配置信息
      ots[i].testGetOmaConfigsNormal() ;
      
      // 测试获取Oma配置信息异常
      ots[i].testGetOmaConfigsAbnormal() ;
      
      // 测试设置Oma配置信息
      ots[i].testSetOmaConfigs() ;
   }
}

main()