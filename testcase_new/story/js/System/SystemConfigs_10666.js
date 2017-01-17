/******************************************************************************
*@Description : test js object System function: getSystemConfigs
*               TestLink : 10666 System对象获取系统配置信息
*@author      : Liang XueWang
******************************************************************************/

// 测试获取系统配置信息
SystemTest.prototype.testGetSystemConfigs = function()
{
   this.init() ;
   
   var type = [ "kernel", "vm", "fs", "debug", "dev", "abi", "all" ] ;
   for( var i = 0;i < type.length;i++ )
   {
      var configObj = this.system.getSystemConfigs( type[i] ).toObj() ;
      
      try
      {
         var command = "sysctl -a 2> /dev/null | grep " + type[i] ;
         var info = this.cmd.run( command ).split( "\n" ) ;
      }
      catch( e )
      {
         if( e == 1 && isEmptyObject( configObj ) )
            ;
         else
         {
            println( "run command " + command ) ;
            throw buildException( "testGetSystemConfigs", e, "get " + type[i], 0, e ) ;
         }
      }
      var obj = {} ;
      for( var i = 0;i < info.length-1;i++ )
      {
         var index = info[i].indexOf( "=" ) ;     // eg: kernel.sem = 250	32000	32	128
         var key = info[i].slice( 0, index-1 ) ;
         var value = info[i].slice( index+2 ) ;
         value = value.replace( /\t/g, ' ' ) ;    // 制表符替换成空格
         value = value.replace( / +/g, ',' ) ;    // 空格替换成逗号
         obj[key] = value ;
      }
      for( var k in configObj )
      {
         if( k.indexOf( "random" ) != -1 )    // 排除随机生成值的字段
            ;
         else if( configObj[k] != obj[k] )
         {
            throw buildException( "testGetSystemConfigs", null, 
                  "test key: " + k + " " + this, obj[k], configObj[k] ) ;
         }   
      }
   }
   
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
      // 测试获取系统配置信息
      // sts[i].testGetSystemConfigs() ;
   }
}

main() ;