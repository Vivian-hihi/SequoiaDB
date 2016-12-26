/******************************************************************************
*@Description : common function for js object System
*@auhor       : Liang XueWang
******************************************************************************/

function SystemTest( hostName, cmSvcName )
{
   if( hostName == undefined )
      this.hostname = COORDHOSTNAME ;
   else
      this.hostname = hostName ;
   if( cmSvcName == undefined )
      this.svcname = CMSVCNAME ;
   else
      this.svcname = cmSvcName ;
   var db = new Sdb( this.hostname, COORDSVCNAME ) ;
   this.isStandalone = commIsStandalone( db ) ;
   db.close() ;
}

SystemTest.prototype.toString = function()
{
   return ( "SystemTest: hostname=" + this.hostname + " svcname=" + this.svcname ) ;
}

SystemTest.prototype.init = function()
{
   if( this.hostname == COORDHOSTNAME || this.hostname == toolGetLocalhost() )
   {
      this.system = System ;
      this.cmd = new Cmd() ;
   }
   else
   {
      this.remote = new Remote( this.hostname, this.svcname ) ;
      this.system = this.remote.getSystem() ;
      this.cmd = this.remote.getCmd() ;
   }
}

SystemTest.prototype.release = function()
{
   if( this.remote != undefined )
      this.remote.close() ;
}

/******************************************************************************
*@Description : check two number is approximately equal to each other or not
*@author      : Liang XueWang
******************************************************************************/
function isApproEqual( n1, n2 )  // n1 n2 >= 0
{
   var max = n1 > n2 ? n1 : n2 ;
   var min = ( max == n1 ) ? n2 : n1 ;
   if( min == 0 ) return max <= 2 ;
   else return min / max >= 0.9 ;
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
*@Description : check machine is ppc or not
*@author      : Liang XueWang            
******************************************************************************/
function isPPC( hostName, cmSvcName )
{
   var remote = new Remote( hostName, cmSvcName ) ;
   var cmd = remote.getCmd() ;
   var info ;
   
   info = cmd.run( "uname -m" ).split( "\n" )[0] ;
   
   remote.close() ;
   return ( info.indexOf( "ppc") != -1 ) ;    
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
         throw buildException( "toolGetIdleSvcName", e ) ;
      }
   }
   remote.close() ;
   return svcname ;
}

/******************************************************************************
*@Description : get sdbcm user
*@author      : Liang XueWang            
******************************************************************************/
function toolGetSdbcmUser( hostName, cmSvcName )
{
   var remote = new Remote( hostName, cmSvcName ) ;
   var cmd = remote.getCmd() ;
   var command = "ps aux | grep sdbcm | grep -E -v 'grep|sdbcmd' | awk '{print $1}'" ;
   var user = cmd.run( command ).split( "\n" )[0] ;
   return user ;   
}

/******************************************************************************
*@Description : check object is empty or not
*@author      : Liang XueWang            
******************************************************************************/
function isEmptyObject( obj )
{
   for( var k in obj )
      return false ;
   return true ;   
}