/******************************************************************************
*@Description : test js object cmd function: run getCommand getLastRet 
*                                            getLastOut
*               TestLink : 9903 执行命令后获取执行命令、执行结果、返回值
*@author      : Liang XueWang 
******************************************************************************/

// 测试运行正确指令
CmdTest.prototype.testRunNormal = function()
{
   this.init() ;
   
   var result = this.cmd.run( "ls", "." ) ;
   var command = this.cmd.getCommand() ;   // 获取上次执行的命令
   if( command != "ls ." )
   {
      throw buildException( "testRun", null, "test getCommand " + this, 
                            "ls .", command ) ;
   }
   var ret = this.cmd.getLastRet() ;       // 获取上次命令是否执行正常
   if( ret != 0 )
   {
      throw buildException( "testRun", null, "test getLastRet " + this, 
                            "ls .", ret ) ;
   }
   var out = this.cmd.getLastOut() ;       // 获取上次命令执行的返回结果
   if( out != result )
   {
      throw buildException( "testRun", null, "test getLastOut " + this, 
                            result, out ) ;
   }
   
   this.release() ;
}

// 测试运行错误指令
CmdTest.prototype.testRunAbnormal = function()
{
   this.init() ;
   
   try
   {
      this.cmd.run( "led", "." ) ;
   }
   catch( e )
   {
      if( e != 127 )
         throw buildException( "testRunAbnormal", e, 
                               "run abnormal command " + this, 127, e ) ;
   }
   var command = this.cmd.getCommand() ;
   if( command != "led ." )
   {
      throw buildException( "testRunAbnormal", null, "test getCommand " + this, 
                            "led .", command ) ;
   }
   var ret = this.cmd.getLastRet() ;
   if( ret != 127 )
   {
      throw buildException( "testRunAbnormal", null, "test getLastRet " + this, 
                            127, ret ) ;
   }
   var out = this.cmd.getLastOut() ;
   if( out.indexOf( "not found" ) == -1 )
   {
      throw buildException( "testRunAbnormal", null, "test getLastOut " + this, 
                            "not found", out ) ;
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
      // 测试运行指令
      cts[i].testRunNormal() ;
      cts[i].testRunAbnormal() ;
   }
}

main()