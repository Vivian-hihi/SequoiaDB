/******************************************************************************
*@Description : test js object File function: init file with mode
*               TestLink : 10899 创建file对象时指定权限
*@auhor       : Liang XueWang
******************************************************************************/

// 测试本地文件对象初始化时指定权限
function testInitLocal()
{
   var filename = "/tmp/testfile.txt" ;
   var modeNumber = [ 0755, 0x1ED, 493 ] ;
   var modeString = "-rwxr-xr-x" ;
   var command = "ls -l " + filename + " | awk '{print $1}'" ;
   var cmd = new Cmd() ;
   cmd.run( "rm -rf " + filename ) ;
   
   for( var i = 0;i < modeNumber.length;i++ )
   {
      var localfile = new File( filename, modeNumber[i] ) ;
      var filemode = cmd.run( command ).split( "\n" )[0] ;
      if( filemode != modeString )
      {
         throw buildException( "testInitLocal", null, "file " + filename, 
                               modeString, filemode ) ;
      }
      cmd.run( "rm -rf " + filename ) ;
   }
}

// 测试远程文件初始化指定权限
function testInitRemote()
{
   var remotehost = toolGetRemotehost() ;
   var remote = new Remote( remotehost, CMSVCNAME ) ;
   var cmd = remote.getCmd() ;
   
   var filename = "/tmp/testfile.txt" ;
   var modeNumber = [ 0755, 0x1ED, 493 ] ;
   var modeString = "-rwxr-xr-x" ;
   var command = "ls -l " + filename + " | awk '{print $1}'" ;
   cmd.run( "rm -rf " + filename ) ;
   for( var i = 0;i < modeNumber.length;i++ )
   {
      var remotefile = remote.getFile( filename, modeNumber[i] ) ;
      var filemode = cmd.run( command ).split( "\n" )[0] ;
      if( filemode != modeString )
      {
         throw buildException( "testInitRemote", null, "file " + filename, 
                               modeString, filemode ) ;
      }
      cmd.run( "rm -rf " + filename ) ;
   }
}

function main()
{
   // 测试本地文件初始化指定权限
   testInitLocal() ;
   // 测试远程文件初始化指定权限
   testInitRemote() ;
   
}

main()