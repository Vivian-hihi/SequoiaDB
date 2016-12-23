/******************************************************************************
*@Description : common function for js object Cmd               
*@author      : Liang XueWang
******************************************************************************/

function CmdTest( hostName, cmSvcName )
{
   if( hostName == undefined )
      this.hostname = COORDHOSTNAME ;
   else
      this.hostname = hostName ;
   if( cmSvcName == undefined )
      this.svcname = CMSVCNAME ;
   else
      this.svcname = cmSvcName ;
}

CmdTest.prototype.toString = function()
{
   return ( "CmdTest: hostname=" + this.hostname + " svcname=" + this.svcname ) ;
}

CmdTest.prototype.init = function()
{
   this.isLocal = this.hostname == COORDHOSTNAME || 
                  this.hostname == toolGetLocalhost() ;
   if( this.isLocal )
   {
      this.cmd = new Cmd() ;
   }
   else
   {
      this.remote = new Remote( this.hostname, this.svcname ) ;
      this.cmd = this.remote.getCmd() ;
   } 
}

CmdTest.prototype.release = function()
{
   if( this.remote != undefined )
      this.remote.close() ;
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
   if( commIsStandalone( db ) )
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
*@Description : get a remote hostname in cluster,
*               if cluster has no remote host, return localhost
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