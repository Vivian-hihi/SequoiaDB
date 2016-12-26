/******************************************************************************
*@Description : test js object File function: find
*               TestLink : 10816 查找File对象的路径
*@auhor       : Liang XueWang
******************************************************************************/

// 测试查找文件
FileTest.prototype.testFind = function()
{
   this.init() ;
   
   var tmpObj = toolGetCmUserGroup( this.cmd ) ;
   var user = tmpObj.user ;
   var group = tmpObj.group ;
   var values = [ "sdbcm.conf", user, "0755", group ] ;  
   var modes = [ "n", "u", "p", "g" ] ;   // 按文件名、用户、文件权限、用户组查找
   var args = [ "-name", "-user", "-perm", "-group" ] ;
   var path = commGetInstallPath() + "/conf" ;
   
   var options = [] ;    // 查找选项
   var commands = [] ;   // 查找命令
   for( var i = 0;i < 4;i++ )
   {
      options[i] = {} ;
      options[i].value = values[i] ;
      options[i].mode = modes[i] ;
      options[i].pathname = path ;
      commands[i] = "find " + path + " " + args[i] + " " + values[i] ;
   }
   
   for( var i = 0;i < options.length;i++ )
   {
      var result = this.file.find( options[i] ).toArray() ;  // 查找文件
      checkFindResult( result, this.cmd, commands[i] ) ;
   }
   
   this.release() ;      
}

/******************************************************************************
*@Description : check find result
*@author      : Liang XueWang            
******************************************************************************/
function checkFindResult( result, cmd, commands )
{
   if( result.length == 0)
   {
      try
      {
         cmd.run( commands ) ;
         throw "find not exist file should be failed" ;
      }
      catch( e )
      {
         if( e != 1 )
         {
            throw buildException( "checkFindResult", e, 
                                  "find no file " + commands, 1, e ) ;
         }
      }
   }
   else
   {
      var findfiles = cmd.run( commands ).split( "\n" ) ;
      for( var j = 0;j < result.length;j++ )
      {
         var fileObj = JSON.parse( result[j] ) ;
         if( fileObj.pathname != findfiles[j] )
         {
            throw buildException( "checkFindResult", null, 
                  "find files " + commands, findfiles, result ) ;
         }    
      }
   }
}

function main()
{
   // 获取本地主机和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var filename = "/tmp/testfile.txt" ;
   var ft1 = new FileTest( localhost, CMSVCNAME ) ;     // 本地File类类型
   var ft2 = new FileTest( localhost, CMSVCNAME, filename ) ;  // 本地file对象
   var ft3 = new FileTest( remotehost, CMSVCNAME ) ;    // 远程File类类型
   var ft4 = new FileTest( remotehost, CMSVCNAME, filename ) ;  // 远程file对象
   
   var fts = [ ft1, ft2, ft3, ft4 ] ;
   
   for( var i = 0; i < fts.length;i++ )
   {
      // 测试获取file对象信息
      fts[i].testFind() ;
   }
}

main()