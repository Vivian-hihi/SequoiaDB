/******************************************************************************
*@Description : common function for js object Oma               
*@author      : Liang XueWang
******************************************************************************/

function OmaTest( hostName, cmSvcName, isLegalHost, isLegalSvc )
{
   if( hostName === undefined )
      this.hostname = COORDHOSTNAME ;
   else
      this.hostname = hostName ;
   if( cmSvcName === undefined )
      this.svcname = CMSVCNAME ;
   else
      this.svcname = cmSvcName ;
   if( isLegalHost === undefined )
      this.islegalhost = true ;
   else
      this.islegalhost = isLegalHost ;
   if( isLegalSvc === undefined )
      this.islegalsvc = true ;
   else
      this.islegalsvc = isLegalSvc ;
   if( this.islegalhost )
   {
      var db = new Sdb( this.hostname, COORDSVCNAME ) ;
      this.isStandalone = commIsStandalone( db ) ;
      db.close() ;
   }
}

OmaTest.prototype.toString = function()
{
   return ( "OmaTest: hostname=" + this.hostname + " svcname=" + this.svcname ) ;
}

OmaTest.prototype.testInit = function() 
{
   try
   {
      this.oma = new Oma( this.hostname, this.svcname ) ;
   }
   catch( e )
   {
      if( !this.islegalhost && e === -15 )
         ;
      else
      {
         throw buildException( "testInit", e, "init oma " + this, "0 -15", e ) ;
      }
   } 
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
         if( e === 1 )
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
      var tmpObj = JSON.parse( tmpInfo[i] ) ;
      var tmpArr = tmpObj.Group ;
      for( var j = 0;j < tmpArr.length;j++ )
      {
         if( hosts.indexOf( tmpArr[j].HostName ) === -1 )
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
      if( hosts[i] !== localhost )
      {
         remotehost = hosts[i] ;
         break ;
      }
   }
   return remotehost ;
}

/******************************************************************************
*@Description : check sdbom exist or not
*@author      : Liang XueWang            
******************************************************************************/
function isOmExist( hostName, cmSvcName )
{
   var oma = new Oma( hostName, cmSvcName ) ;
   var rc ;
   
   var arr = oma.listNodes( { type: "om" } ).toArray() ;
   if( arr.length !== 0 )
      rc = true ;
   else
      rc = false ;
   
   oma.close() ;
   return rc ;
}

/******************************************************************************
*@Description : check getOmaConfigs/getNodeConfigs result
*@author      : Liang XueWang              
******************************************************************************/
function checkResult( info, content, func )
{ 
   for( var i in info )
   {
      var found = false ;
      for( var j = 0;j < content.length;j++ )
      {
         content[j] = content[j].replace( / /g,"" ) ;
         if( content[j][0] == "#" )
            continue ;
         var ind = content[j].indexOf( i ) ;
         if( ind == -1 )
            continue ;
         found = true ;
         var value1 = content[j].slice( ind+i.length+1 ).toLowerCase() ;
         var value2 = info[i].toString().toLowerCase() ;
         if( value1 != value2 )
            throw buildException( "checkResult", null, func + " i=" + i, value1, value2 ) ;   
      }
      if( found === false )
         throw buildException( "checkResult", func + ", i=" + i ) ;   
   }
}

/******************************************************************************
*@Description : get sequoiadb dir eg: /opt/sequoiadb/bin/.. /opt/sequoiadb/
*@author      : Liang XueWang              
******************************************************************************/
function toolGetSequoiadbDir( hostname, svcname )
{
   var dir = [] ;
   var remote = new Remote( hostname, svcname ) ;
   var system = remote.getSystem() ;
   var tmp = system.getEWD() ;
   var ind = tmp.indexOf( "/bin" ) ;
   dir[0] = tmp + "/.." ;
   dir[1] = tmp.slice( 0, ind ) ;
   remote.close() ;
   
   if( hostname === COORDHOSTNAME || hostname === toolGetLocalhost() )
   {
      system = System ;
      tmp = system.getEWD() ;
      ind = tmp.indexOf( "/bin" ) ;
      dir[2] = tmp + "/.." ;
      dir[3] = tmp.slice( 0, ind ) ;
   }
      
   return dir ;
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