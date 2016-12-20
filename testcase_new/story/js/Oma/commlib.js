/******************************************************************************
*@Description : common function for js object Oma               
*@author      : Liang XueWang
******************************************************************************/

function OmaTest( HostName, CmSvcName, isLegalHost, isLegalSvc )
{
   if( HostName == undefined )
      this.hostname = COORDHOSTNAME ;
   else
      this.hostname = HostName ;
   if( CmSvcName == undefined )
      this.svcname = CMSVCNAME ;
   else
      this.svcname = CmSvcName ;
   if( isLegalHost == undefined )
      this.islegalhost = true ;
   else
      this.islegalhost = isLegalHost ;
   if( isLegalSvc == undefined )
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
   println( "OmaTest: hostname=" + this.hostname + " svcname=" + this.svcname ) ;
}

OmaTest.prototype.testInit = function() 
{
   try
   {
      this.oma = new Oma( this.hostname, this.svcname ) ;
   }
   catch( e )
   {
      if( !this.islegalhost && e == -15 )
         ;
      else
      {
         throw buildException( "testInit", e ) ;
      }
   } 
}

/******************************************************************************
*@Description : get a idle svcname
*@author      : Liang XueWang            
******************************************************************************/
function toolGetIdleSvcName( HostName, CmSvcName )
{
   var remote = new Remote( HostName, CmSvcName ) ;
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

/******************************************************************************
*@Description : get hosts in cluster
*@author      : Liang XueWang            
******************************************************************************/
function toolGetHosts()
{
   var Hosts = [] ;
   var k = 0 ;
   
   var db = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
   if( commIsStandalone(db) )
   {
      println( "Run mode is standalone." ) ;
      db.close() ;
      return Hosts ;
   }
   
   var tmpInfo = db.listReplicaGroups().toArray() ;
   for( var i = 0;i < tmpInfo.length;i++ )
   {
      var tmpObj = db.eval( "(" + tmpInfo[i] + ")" ).toObj() ;
      var tmpArr = tmpObj.Group ;
      for( var j = 0;j < tmpArr.length;j++ )
      {
         if( Hosts.indexOf( tmpArr[j].HostName ) == -1 )
            Hosts[k++] = tmpArr[j].HostName ;
      }
   }
   db.close() ;
   return Hosts ;
}

/******************************************************************************
*@Description : get local hostname
*@author      : Liang XueWang            
******************************************************************************/
function toolGetLocalhost()
{
   var cmd = new Cmd() ;
   var Localhost = cmd.run( "hostname" ).split( "\n" )[0] ;
   return Localhost ;
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

/******************************************************************************
*@Description : check sdbom exist or not
*@author      : Liang XueWang            
******************************************************************************/
function isOmExist( HostName, CmSvcName )
{
   var oma = new Oma( HostName, CmSvcName ) ;
   var rc ;
   
   var arr = oma.listNodes( { type: "om" } ).toArray() ;
   if( arr.length != 0 )
      rc = true ;
   else
      rc = false ;
   
   oma.close() ;
   return rc ;
}

/******************************************************************************
*@Description : check getOmaInstallInfo/getOmaConfigs/getNodeConfigs result
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
         var ind = content[j].indexOf( i ) ;
         if( ind == -1 )
            continue ;
         found = true ;
         var value1 = content[j].slice( ind+i.length+1 ).toLowerCase() ;
         var value2 = info[i].toString().toLowerCase() ;
         if( value1 != value2 )
            throw buildException( "checkResult", 0, func + " i=" + i, value1, value2 ) ;   
      }
      if( found == false )
         throw buildException( "checkResult", func + ", i=" + i ) ;   
   }
}