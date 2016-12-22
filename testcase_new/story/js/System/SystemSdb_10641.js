/******************************************************************************
*@Description : test js object System function: getPID getTID getEWD
*               TestLink : 10641 System对象获取sdb的进程ID
*                          10642 System对象获取sdb的线程ID
*                          10643 System对象获取sdb可执行程序的路径          
*@author      : Liang XueWang
******************************************************************************/

// 测试获取sdb shell进程ID
SystemTest.prototype.testGetPID = function()
{
   this.init() ;
   
   var pid = this.system.getPID() ;
   var command ;
   if( this.system == System )
      command = "pgrep '^sdb$'" ;
   else
      command = "pgrep '^sdbcm$'" ;
   var pids = this.cmd.run( command ) ;
   if( pids.indexOf( pid ) == -1 )
   {
      throw buildException( "testGetPID", null, "get PID " + this, pids, pid ) ;
   }
   
   this.release() ;
}

// 测试获取sdb shell线程ID
SystemTest.prototype.testGetTID = function()
{
   this.init() ;
   
   var pid = this.system.getPID() ;
   var tid = this.system.getTID() ;
   var tids = this.cmd.run( "ps -T -p " + pid + " | awk '{print $2}'" ).split("\n") ;
   if( tids.indexOf( "" + tid ) == -1 )
   {
      throw buildException( "testGetTID", null, "get TID " + this, tid, tids ) ;
   }
   
   this.release() ;
}

// 测试获取sdb所在的工作目录   
SystemTest.prototype.testGetEWD = function()
{
   this.init() ;
   
   var InstallPath = commGetInstallPath() ;
   var WorkDir = this.system.getEWD() ;
   if( WorkDir != InstallPath + "/bin" )
   {
      throw buildException( "testGetEWD", null, "get EWD " + this, 
                            InstallPath + "/bin", WorkDir ) ;
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
      // 测试获取sdb进程ID
      sts[i].testGetPID() ;
      
      // 测试获取sdb线程ID
      sts[i].testGetTID() ;
      
      // 测试获取sdb目录
      sts[i].testGetEWD() ;
   }
}

main()