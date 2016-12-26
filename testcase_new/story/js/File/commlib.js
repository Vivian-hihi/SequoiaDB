/******************************************************************************
*@Description : common function for js object File
*@auhor       : Liang XueWang
******************************************************************************/

function FileTest( hostName, cmSvcName, fileName )
{
   if( hostName == undefined )
      this.hostname = COORDHOSTNAME ;
   else
      this.hostname = hostName ;   // 主机名    
   if( cmSvcName == undefined )
      this.svcname = CMSVCNAME ;
   else
      this.svcname = cmSvcName ;   // 端口号  
   this.filename = fileName ;      // 文件名
}

FileTest.prototype.init = function()
{
   this.isLocal = false ;          // 是否连接本地cm
   if( this.hostname == toolGetLocalhost() || this.hostname == COORDHOSTNAME )
      this.isLocal = true ;
      
   if( this.isLocal )
   {
      this.cmd = new Cmd() ;       // 本地cmd对象
      if( this.filename == undefined )
         this.file = File ;                           // 本地File类类型
      else
         this.file = new File( this.filename ) ;      // 本地file对象
   }
   else
   {
      this.remote = new Remote( this.hostname, this.svcname ) ;
      this.file = this.remote.getFile( this.filename ) ;   // 远程File类类型或远程file对象
      this.cmd = this.remote.getCmd() ;   // 远程cmd对象
   }
}

FileTest.prototype.release = function()
{
   this.cmd.run( "rm -rf " + this.filename ) ;    // 删除文件
   if( this.remote != undefined )
   {
      this.remote.close() ;    // 断开连接
   }  
}

FileTest.prototype.toString = function()
{
   return ( "FileTest: hostname=" + this.hostname + " svcname=" + this.svcname +
            " filename=" + this.filename ) ;
}
   
/******************************************************************************
*@Description : get hosts in cluster
*@author      : Liang XueWang            
******************************************************************************/
function toolGetHosts()
{
   var hosts = [] ;
   var k = 0 ;
   
   var db = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
   if( commIsStandalone(db) )
   {
      println( "Run mode is standalone." ) ;
      db.close() ;
      return hosts ;
   }
   
   var tmpInfo = db.listReplicaGroups().toArray() ;
   for( var i = 0;i < tmpInfo.length;i++ )
   {
      var tmpObj = db.eval( "(" + tmpInfo[i] + ")" ).toObj() ;
      var tmpArr = tmpObj.Group ;
      for( var j = 0;j < tmpArr.length;j++ )
      {
         if( hosts.indexOf( tmpArr[j].HostName ) == -1 )
            hosts[k++] = tmpArr[j].HostName ;
      }
   }
   db.close() ;
   return hosts ;
}


/******************************************************************************
*@Description : get local hostname
*@author      : Liang XueWang            
******************************************************************************/
function toolGetLocalhost()
{
   var cmd = new Cmd() ;
   var localhost = cmd.run( "hostname" ).split( "\n" )[0] ;
   return localhost ;
}

/******************************************************************************
*@Description : get a remote hostname in cluster
*               if cluster has no remote host,return localhost
*@author      : Liang XueWang
******************************************************************************/
function toolGetRemotehost()
{
   var hosts = toolGetHosts() ;
   var localhost = toolGetLocalhost() ;
   var remotehost = localhost ;
   for( var i = 0;i < hosts.length;i++ )
   {
      if( hosts[i] != localhost )
      {
         remotehost = hosts[i] ;
         break ;
      }
   }
   return remotehost ;
}

/******************************************************************************
*@Description : get sdbcm user
*@author      : Liang XueWang            
******************************************************************************/
function toolGetSdbcmUser( hostName, cmSvcName )
{
   var remote = new Remote( hostName, cmSvcName ) ;
   var cmd = remote.getCmd() ;
   var command = "ps aux | grep sdbcm | grep -E -v 'grep|sdbcmd' |" +
                 " awk '{print $1}'" ;
   var user = cmd.run( command ).split( "\n" )[0] ;
   return user ;   
}

/******************************************************************************
*@Description : get current user whoami
*@author      : Liang XueWang            
******************************************************************************/
function toolGetCurrentUser( hostName, cmSvcName )
{
   var remote = new Remote( hostName, cmSvcName ) ;
   var cmd = remote.getCmd() ;
   var user = cmd.run( "whoami" ).split( "\n" )[0] ;
   return user ;
}

/******************************************************************************
*@Description : get user and group  sdbcm.conf
*@author      : Liang XueWang            
******************************************************************************/
function toolGetCmUserGroup( cmd )
{
   var InstallPath = commGetInstallPath() ;
   var file = InstallPath + "/conf/sdbcm.conf" ;
   var command = "ls -l " + file + " | awk '{print $3,$4}'" ;
   var tmpInfo = cmd.run( command ).split( "\n" )[0] ;
   var tmp = tmpInfo.split( " " ) ;
   var result = {} ;
   result["user"] = tmp[0] ;
   result["group"] = tmp[1] ;
   return result ;
}