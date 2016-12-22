/******************************************************************************
*@Description : test js object Remote initialization
*               TestLink : 10636 初始化Remote对象，端口不存在
*                          10637 初始化Remote对象，端口不是cm端口
*@author      : Liang XueWang
******************************************************************************/

// 测试使用非cm端口初始化remote创建system对象后，调用type方法出错
RemoteTest.prototype.testInitWithWrongSvc = function()
{
   this.testInit() ;
   var system = this.remote.getSystem() ;
   
   // 测试调用type方法时报错
   try
   {
      system.type() ;
      throw "illegal system call type should be failed" ;
   }
   catch( e )
   {
      if( e != -6 )
      {
         throw buildException( "testInitWithWrongSvc", e, 
               "check init remote with COORDSVCNAME " + this, -6, e ) ;
      }   
   }
   
   this.remote.close() ;
}

function main()
{
   // 获取远程主机
   var remotehost = toolGetRemotehost() ;
   
   // 测试使用不存在的主机初始化
   var rt = new RemoteTest( "IllegalHost", CMSVCNAME, false ) ;
   rt.testInit() ;
   
   // 获取空闲端口
   var svcname = toolGetIdleSvcName( remotehost, CMSVCNAME ) ;
   
   // 测试使用不存在的端口初始化
   rt = new RemoteTest( remotehost, svcname, true, false ) ;
   rt.testInit() ;
   
   // 测试使用非cm端口初始化（初始化及创建对象时不报错，调用方法时报错）
   rt = new RemoteTest( remotehost, COORDSVCNAME ) ;
   rt.testInitWithWrongSvc() ;
}

main()