/******************************************************************************
*@Description : common function for test getSlave
*               
*@auhor       : Liang XueWang
******************************************************************************/

/*******************************************************************
* @Description : check path has / in the end or not
*                add / if not
* @author      : Liang XueWang
*
********************************************************************/
function adaptPath( path )
{
   if( path.lastIndexOf( '/' ) !== path.length-1 )
      path += '/' ;
   return path ;
}

/*******************************************************************
* @Description : create nodes in rg
*                hostname : local hostname, 
*                svcname  : RSRVPORTBEGIN ....
*                dbpath   : RSRVNODEDIR + "data/" + svcname
* @author      : Liang XueWang
*
********************************************************************/
function createNodes( rg, nodeNum )
{
   for( var i = 0;i < nodeNum;i++ )
   {
      try
      {
         var host = System.getHostName() ;
         var svc = parseInt( RSRVPORTBEGIN ) + i*10 ;
         var dbpath = adaptPath( RSRVNODEDIR ) + "data/" + svc ;
         println( "create node " + host + ":" + svc + " " + dbpath ) ;
         rg.createNode( host, svc, dbpath ) ;
      }
      catch( e )
      {
         throw buildException( "createNodes", e, "create node", 0, e ) ;
      }
   }
}

/**********************************************************************
 * @Description : get nodes in group
 *                rgName: group name, ex "group1"
 *                return nodes array, ex [ "sdbserver1:20100", .... ]
 * @author      : Liang XueWang
 *
 **********************************************************************/
function getGroupNodes( db, rgName )
{
    var arr = new Array() ;
    var tmpObj = db.getRG( rgName ).getDetail().next().toObj() ;
    var tmpGroupArray = tmpObj["Group"] ;
    for( var j = 0;j < tmpGroupArray.length;++j )
    {
        var tmpNodeObj = tmpGroupArray[j] ;
        var hostName = tmpNodeObj["HostName"] ;
        for( var k = 0;k < tmpNodeObj.Service.length;++k )
        {
            var tmpSvcObj = tmpNodeObj.Service[k] ;
            if( tmpSvcObj["Type"] == 0 )
            {
                nodeName = hostName + ":" + tmpSvcObj["Name"] ;
                arr.push( nodeName ) ;
                break ;
            }
        }
    }

    return arr ;
}

/**********************************************************************
 * @Description : check group has master or not
 *                
 * @author      : Liang XueWang
 *
 **********************************************************************/
function isMasterExist( db, rgName )
{
   var clName = "testHasMasterCl" ;
   var hasMaster = false ;
   try
   {
      commCreateCLByOption( db, COMMCSNAME, clName, { Group: rgName } ) ;
      hasMaster = true ;
      commDropCL( db, COMMCSNAME, clName ) ;
   }
   catch( e )
   {
      if( e !== -104 )
      {
         throw buildException( "isMasterExist", e, "create cl", "0 -104", e ) ;
      }
   }
   return hasMaster ;
}