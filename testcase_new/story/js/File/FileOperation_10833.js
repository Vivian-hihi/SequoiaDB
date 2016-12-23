/******************************************************************************
*@Description : test js object File function: mkdir move copy remove
*               TestLink : 10837 新建目录
*                          10836 移动文件
*                          10835 拷贝文件
*                          10833 删除文件
*@auhor       : Liang XueWang
******************************************************************************/

// 测试创建目录，移动文件，拷贝文件，删除文件
FileTest.prototype.testFileOperation = function()
{
   this.init() ;
   
   var tmpDirName = "/tmp/FileTest" ;
   var tmpFileName = "/tmp/FileTest/tmpFile" ;
   var tmpFile ;
   
   this.file.mkdir( tmpDirName ) ;   // 创建目录
   checkMkdir( this.cmd, tmpDirName ) ;
   if( this.isLocal )
      tmpFile = new File( tmpFileName ) ;
   else
      tmpFile = this.remote.getFile( tmpFileName ) ;
      
   this.file.move( tmpFileName, tmpFileName + ".move" ) ; // 移动文件
   checkMove( this.cmd, tmpFileName, tmpFileName + ".move" ) ;
   this.file.move( tmpFileName + ".move", tmpFileName ) ; 
   
   this.file.copy( tmpFileName, tmpFileName + ".copy" ) ;  // 拷贝文件
   checkCopy( this.cmd, tmpFileName, tmpFileName + ".copy" ) ;
   
   this.file.remove( tmpFileName ) ;      // 删除文件
   checkRemove( this.cmd, tmpFileName ) ;
   this.file.remove( tmpFileName + ".copy" ) ;
   this.file.remove( tmpDirName ) ; 
   
   this.release() ;  
}

/******************************************************************************
*@Description : check mkdir
*@author      : Liang XueWang            
******************************************************************************/
function checkMkdir( cmd, dirName )
{
   try
   {
      cmd.run( "ls -al " + dirName ) ;
   }
   catch( e )
   {
      throw buildException( "checkMkdir", e ) ;
   }
}

/******************************************************************************
*@Description : check move file
*@author      : Liang XueWang            
******************************************************************************/
function checkMove( cmd, oldFile, newFile )
{
   try
   {
      cmd.run( "ls -al " + oldFile ) ;
      throw "list moved file should be failed" ;
   }
   catch( e )
   {
      if( e != 2 )
         throw buildException( "checkMove", e, "list " + oldFile, 2, e ) ;
   }
   try
   {
      cmd.run( "ls -al " + newFile ) ;
   }
   catch( e )
   {
      throw buildException( "checkMove", e, "list " + newFile, 0, e ) ;   
   }   
}

/******************************************************************************
*@Description : check copy file
*@author      : Liang XueWang            
******************************************************************************/
function checkCopy( cmd, srcFile, dstFile )
{
   try
   {
      cmd.run( "ls -al " + srcFile ) ;
      cmd.run( "ls -al " + dstFile ) ;
   }
   catch( e )
   {
      throw buildException( "checkCopy", e, "check " + srcFile + " " + dstFile, 0, e ) ;
   }
}

/******************************************************************************
*@Description : check remove file
*@author      : Liang XueWang            
******************************************************************************/
function checkRemove( cmd, fileName )
{
   try
   {
      cmd.run( "ls -al " + fileName ) ;
      throw "list removed file should be failed" ;
   }
   catch( e )
   {
      if( e != 2 )
         throw buildException( "checkRemove", e ) ;
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
      // 测试创建目录，移动文件，复制文件，删除文件
      fts[i].testFileOperation() ;
   }
}

main()