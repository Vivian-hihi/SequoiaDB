/******************************************************************************
*@Description : test js object cmd function: runJS
*               TestLink : 9901 执行JS语句 
*@author      : Liang XueWang 
******************************************************************************/

// 测试运行JS命令
CmdTest.prototype.testRunJS = function()
{
   this.init() ;
   
   try
   {
      var result = this.cmd.runJS( "1 + 2 * 3" ) ;
   }
   catch( e )
   {
      if( e == -10 && this.isLocal )
         ;
      else
         throw buildException( "testRunJS", e, "run js " + this, 0, e ) ;
   }
   if( result != undefined && result != "7" )
   {
      throw buildException( "testRunJS", null, "check result " + this, 
                            "7", result ) ;
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
      cts[i].testRunJS() ;
   }
}

main()