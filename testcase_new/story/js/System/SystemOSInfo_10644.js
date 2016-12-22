/******************************************************************************
*@Description : test js object System function: getPID getTID getEWD type
*               getReleaseInfo getIpTablesInfo getHostName ping  
*               TestLink : 10644 获取System对象信息 
*                          10689 System对象获取主机名
*                          10672 System对象检查网络是否联通
*                          10673 System对象查看系统类型
*                          10674 System对象获取发行信息
*                          10688 System对象获取IpTables信息 
*@author      : Liang XueWang
******************************************************************************/

// 测试获取system对象信息
SystemTest.prototype.testGetInfo = function()
{
   this.init() ;
   if( this.system == System )
   {
      println( "System has no function getInfo" ) ;
      return ;
   }
   
   var info = this.system.getInfo().toObj() ;
   if( info.type != "System" || 
       info.hostname != this.hostname ||
       info.svcname != this.svcname ||
       info.isRemote != true )
   {
      throw buildException( "testGetInfo", 0, "test system object info", 
            this.hostname + ":" + this.svcname, JSON.stringify(info) ) ;
   }
   
   this.release() ;
}

// 测试获取主机名
SystemTest.prototype.testGetHostName = function()
{
   this.init() ;
      
   var hostname1 = this.system.getHostName() ;
   var hostname2 = this.cmd.run( "hostname" ).split( "\n" )[0] ;
   if( hostname1 != hostname2 )
   {
      throw buildException( "testGetHostName", null, 
                            "get hostname " + this, hostname2, hostname1 ) ;
   }
   
   this.release() ;
}

// 测试网络是否连通
SystemTest.prototype.testPing = function()
{
   this.init() ;
   
   // 测试连通本机
   var obj = this.system.ping( COORDHOSTNAME ).toObj() ;
   if( obj.Target != COORDHOSTNAME || obj.Reachable != true )
   {
      throw buildException( "testPing", null, "ping localhost " + this, 
                            true, obj.Reachable ) ;
   }
   
   // 测试连通不存在的主机
   obj = this.system.ping( "NotExistHost" ).toObj() ;
   if( obj.Target != "NotExistHost" || obj.Reachable != false )
   {
      throw buildException( "testPing", null, "ping not exist host " + this, 
                            false, obj.Reachable ) ;
   }
   
   this.release() ;   
}

// 测试获取系统类型 LINUX WINDOWS
SystemTest.prototype.testType = function()
{
   this.init() ;
   
   var t = this.system.type() ;
   if( t != "LINUX" )
   {
      throw buildException( "testType", null, "test type " + this, "LINUX", t ) ;
   }
   
   this.release() ;
}

// 测试获取系统发行版本信息
SystemTest.prototype.testGetReleaseInfo = function()
{
   this.init() ;
   
   // 测试获取的系统发行版本信息
   var descript1 = this.system.getReleaseInfo().toObj().Description ;
   descript1 = descript1.replace( /[\t ]/g, '' ) ;
   var command = "lsb_release -a | grep Description | awk -F ':' '{print $2}'" ;
   var tmpInfo = this.cmd.run( command ).split( "\n" ) ;
   var descript2 = tmpInfo[0] ;
   if( descript2 == "No LSB modules are available." )
   {
       descript2 = tmpInfo[1] ; 
   } 
   descript2 = descript2.replace( /[\t ]/g, '' ) ;
   if( descript1 != descript2 )
   {
      throw buildException( "testGetReleaseInfo", null, "test description " + this, 
                            descript2, descript1 ) ;
   }
   
   // 测试获取的系统位数
   var bit1 = this.system.getReleaseInfo().toObj().Bit ;
   var bit2 = this.cmd.run( "getconf LONG_BIT" ).split( "\n" )[0] ;
   if( bit1 != bit2 )
   {
      throw buildException( "testGetReleaseInfo", null, "test bit " + this, bit2, bit1 ) ;
   }
   
   this.release() ;
}

// 测试获取防火墙信息，返回 { FireWall: "unknown" }   
SystemTest.prototype.testGetIpTablesInfo = function()
{
   this.init() ;
   
   var iptables = this.system.getIpTablesInfo().toObj() ;
   if( iptables.FireWall != "unknown" )
   {
      throw buildException( "testGetIpTablesInfo", 0, "get iptables info " + this, 
                            "unknown", iptables.FireWall ) ;
   }
   
   this.release() ;
}

function main()
{
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var st1 = new SystemTest( localhost, CMSVCNAME ) ;
   var st2 = new SystemTest( remotehost, CMSVCNAME ) ;
   var sts = [ st1, st2 ] ;
   
   for( var i = 0;i < sts.length;i++ )
   {
      // 测试获取操作系统类型
      sts[i].testType() ;
      
      // 测试获取操作系统发行版本
      sts[i].testGetReleaseInfo() ;
      
      // 测试获取防火墙信息
      sts[i].testGetIpTablesInfo() ;
      
      // 测试获取主机名
      sts[i].testGetHostName() ;
      
      // 测试网络连通性
      sts[i].testPing() ;
      
      // 测试获取system对象信息
      sts[i].testGetInfo() ; 
   }
}

main()