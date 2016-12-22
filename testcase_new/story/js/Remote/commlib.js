/******************************************************************************
*@Description : common function for js object Remote
*@auhor       : Liang XueWang
******************************************************************************/

function RemoteTest( hostName, cmSvcName, isLegalHost, isLegalSvc )
{
   if( hostName == undefined )
      this.hostname = COORDHOSTNAME ;
   else
      this.hostname = hostName ;
   if( cmSvcName == undefined )
      this.svcname = CMSVCNAME ;
   else
      this.svcname = cmSvcName ;
   if( isLegalHost == undefined )
      this.islegalHost = true ;
   else
      this.islegalhost = isLegalHost ;
   if( isLegalSvc == undefined )
      this.islegalsvc = true ;
   else
      this.islegalsvc = isLegalSvc ;
}

RemoteTest.prototype.toString = function()
{
   return ( "hostname=" + this.hostname + " svcname=" + this.svcname ) ;
}

RemoteTest.prototype.testInit = function()
{
   try
   {
      this.remote = new Remote( this.hostname, this.svcname ) ;
   }
   catch( e )
   {
      if( ( !this.islegalhost || !this.islegalsvc ) && e == -15 )
         ;
      else
      {
         throw buildException( "testInit", e, "init remote " + this, "0 -15", e ) ;
      }   
   } 
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
*@Description : get a idle svcname
*@author      : Liang XueWang            
******************************************************************************/
function toolGetIdleSvcName( hostName, cmSvcName )
{
   var remote = new Remote( hostName, cmSvcName ) ;
   var cmd = remote.getCmd() ;
     
   var svcname ;
   for( svcname = RSRVPORTBEGIN; svcname <= RSRVPORTEND; svcname = svcname*1 + 5 )
   {
      try
      {
         cmd.run( "netstat -anp | grep " + svcname ) ;
      }
      catch( e )
      {
         if( e == 1 )
         {
            remote.close() ;
            return svcname ;
         }
         throw buildException( "getIdleSvcName", e ) ;
      }
   }
   remote.close() ;
   return svcname ;
}