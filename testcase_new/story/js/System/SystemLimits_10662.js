/******************************************************************************
*@Description : test js object System function: getProcUlimitConfigs 
*                                               setProcUlimitConfigs
*               TestLink : 10662 System对象获取limits配置信息
*                          10663 System对象设置limits信息
*@author      : Liang XueWang
******************************************************************************/

// 测试获取limits信息
SystemTest.prototype.testGetProcUlimitConfigs = function()
{
   this.init() ;
   
   var limits = this.system.getProcUlimitConfigs().toObj() ;
   for( var k in limits )
   {
      var str = k.replace( /_/g, " " ) ;
      if( str == "realtime priority" )
         str = "real-time priority" ;
      var command = "/bin/bash -c 'ulimit -a' | grep " + "'^" + str +"'" ;
      var info = this.cmd.run( command ).split( "\n" )[0] ;
      var limit = info.slice( 37 ) ;
      if( limit == "unlimited" )
         limit = -1 ;
      else if( info.indexOf( "kbytes" ) != -1 )
         limit = 1024 * limit ;
      if( limits[k] != limit )
      {
         throw buildException( "testGetProcUlimits", null, 
                               "get limits of " + k + this, limit, limits[k] ) ;
      }   
   }
   
   this.release() ;
}

// 测试设置limits信息
SystemTest.prototype.testSetProcUlimitConfigs = function()
{
   this.init() ;
   
   var oldLimits = this.system.getProcUlimitConfigs().toObj() ;
   var oldMaxMemorySize = oldLimits.max_memory_size ;
   
   var maxMemSize = [ 9223372036854775808, "9223372036854775808",
                      9223372036854775809, "9223372036854775809",
                      -100, "-100", -1, "-1" ] ;
   var results = [ "9223372036854775808", "9223372036854775808",
                   "9223372036854775808", "9223372036854775809",
                   "18446744073709551516", "18446744073709551516",
                   -1, -1 ] ;
   for( var i = 0;i < maxMemSize.length;i++ )
   {                
      oldLimits.max_memory_size = maxMemSize[i] ;
      this.system.setProcUlimitConfigs( oldLimits ) ;
      var newLimits = this.system.getProcUlimitConfigs().toObj() ;
      if( newLimits.max_memory_size !== results[i] )
      {
         throw buildException( "testSetProcUlimitConfigs", null, 
               "test set ulimit " + this, results[i], newLimits.max_memory_size ) ;
      }
   }
   
   oldLimits.max_memory_size = oldMaxMemorySize ;   
   this.system.setProcUlimitConfigs( oldLimits ) ;
   
   this.release() ;
}


function main()
{
   // 获取本地主机和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var st1 = new SystemTest( localhost, CMSVCNAME ) ;
   var st2 = new SystemTest( remotehost, CMSVCNAME ) ;
   var sts = [ st1, st2 ] ;
   
   for( var i = 0;i < sts.length;i++ )
   {
      // 测试获取limits
      // sts[i].testGetProcUlimitConfigs() ;
      
      // 测试设置limits
      sts[i].testSetProcUlimitConfigs() ;
   } 
   
}
   
main()
