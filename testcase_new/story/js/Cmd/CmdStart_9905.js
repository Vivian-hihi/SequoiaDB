/******************************************************************************
*@Description : test js object cmd function: start
*               TestLink : 9905 后台执行命令 
*@author      : Liang XueWang 
******************************************************************************/

// 测试后台运行命令
CmdTest.prototype.testStart = function()
{
   this.init() ;
   
   var pid = this.cmd.start( "sleep", "3" ) ;
   var command = "ps aux | grep " + pid + " | grep -v grep" ;
   var tasks = this.cmd.run( command ).split( "\n" ) ;
   if( tasks.length != 2 )
   {
      throw buildException( "testStart", null, "list background task " + this, 
                            pid, tasks ) ;
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
      // 测试后台运行指令
      cts[i].testStart() ;
   }
}

main()