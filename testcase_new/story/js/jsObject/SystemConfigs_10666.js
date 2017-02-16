/******************************************************************************
*@Description : test js object System function: getSystemConfigs
*               TestLink : 10666 System对象获取系统配置信息
*@author      : Liang XueWang
******************************************************************************/

// 测试获取系统配置信息
SystemTest.prototype.testGetSystemConfigs = function()
{
   this.init() ;
   
   var type = [ "kernel", "vm", "fs", "debug", "dev", "abi", "net", "all" ] ;
   // 动态变化的字段
   var except = [ "fs.dentry-state", "fs.inode-nr", "fs.inode-state",
                  "fs.file-nr" ] ;
   var allConfig = {} ;
   for( var i = 0;i < type.length;i++ )
   {
      var configObj = this.system.getSystemConfigs( type[i] ).toObj() ;
      var result = {} ;
      if( type[i] !== "all" )
      {
         var obj = toolGetConfigs( this.cmd, type[i] ) ;
         for( var k in obj )
            allConfig[k] = obj[k] ;
         result = obj ;
      }
      else
      {
         result = allConfig ;
      }
      for( var k in configObj )
      {
         // 排除随机生成或动态变化的字段
         if( k.indexOf( "random" ) !== -1 )
            continue ;
         else if( getIndexInArray( k, except ) !== -1 )
            continue ;
         else if( configObj[k] !== result[k] )
         {
            throw buildException( "testGetSystemConfigs", null, 
                  "test key: " + k + " " + this, result[k], configObj[k] ) ;
         }   
      }
   }
   
   this.release() ;
}

// 获取系统配置信息，从/proc/sys目录下的文件中获取
function toolGetConfigs( cmd, type )
{
   var configObj = {} ;
   try
   {
      var command = "find /proc/sys/" + type + " -type f" ;
      var files = cmd.run( command ).split( "\n" ) ;
   }
   catch( e )
   {
      if( e === 1 )
         return configObj ;
      else
      {
         println( "run command " + command ) ;
         throw buildException( "toolGetConfigs", e, "get " + type, 0, e ) ;
      }
   }
   for( var i = 0;i < files.length-1;i++ )
   {
      var filename = files[i] ;
      var tmpInfo = filename.split( "/" ) ;
      var key = "" ;
      for( var j = 0; j < tmpInfo.length; j++ )
      {
         if( tmpInfo[j] === "" || tmpInfo[j] === "proc" ||
             tmpInfo[j] === "sys" )
             continue ;
         key += tmpInfo[j] ;
         if( j !== tmpInfo.length-1 )
            key += "." ;
      }
      try
      {
         var fileContent = cmd.run( "cat " + filename ).split( "\n" ) ;
      }
      catch( e )
      {
         if( e === 1 ) 
            continue ;
         else 
            throw buildException( "toolGetConfigs", e, "cat " + filename, 0, e ) ;
      }
      var value = "" ;
      for( var k = 0;k < fileContent.length-1;k++ )
      {
         if( fileContent[k] === "" ) 
            continue ;
         // 制表符替换成四个空格
         value += fileContent[k].replace( /\t/g, '    ' ) + ";" ;
      }
      // 删除最后一个分号
      value = value.substring( 0, value.length-1 ) ;
      configObj[key] = value ;
   }
   return configObj ;
}

// 查找元素在数组中的下标
function getIndexInArray( a, arr )
{
   var index = -1 ;
   for( var i = 0;i < arr.length;i++ )
   {
      if( arr[i] === a )
      {
         index = i ;
         break ;
      }
   }
   return index ;
}

function main()
{
   // 获取本地主机和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var localSystem = new SystemTest( localhost, CMSVCNAME ) ;
   var remoteSystem = new SystemTest( remotehost, CMSVCNAME ) ;
   var systems = [ localSystem, remoteSystem ] ;
   
   for( var i = 0;i < systems.length;i++ )
   {
      // 测试获取系统配置信息
      systems[i].testGetSystemConfigs() ;
   }
}

main() ;