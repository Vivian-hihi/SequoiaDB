/*******************************************************************
* @Description : test health snapshot field "Ulimit"
*                seqDB-14433:Ulimit字段测试
* @author      : linsuqiang
* 
*******************************************************************/

main( db );

function main( db )
{
   var cmd = new Cmd();
   var nodeInfo = getLocalNodeInfo( cmd );
   var expUlimit = getUlimitByCmd( cmd, nodeInfo.PID );
   var actUlimit = getUlimitBySnap( db, nodeInfo.GID, nodeInfo.NID );
   assert( isEquals( expUlimit, actUlimit ), "wrong ulimit \n" +
         "expect " + JSON.stringify( expUlimit ) + " \n" +
         "actual " + JSON.stringify( actUlimit ) );
}

function getUlimitByCmd( cmd, PID )
{
   var ulimitJson = {};
   ulimitJson.CoreFileSize    = getUlimitItem( cmd, PID, "Max core file size" );
   ulimitJson.VirtualMemory   = getUlimitItem( cmd, PID, "Max address space" );
   ulimitJson.OpenFiles       = getUlimitItem( cmd, PID, "Max open files" );
   ulimitJson.NumProc         = getUlimitItem( cmd, PID, "Max processes" );
   ulimitJson.FileSize        = getUlimitItem( cmd, PID, "Max file size" );
   for( var key in ulimitJson )
   {
      if( ulimitJson[key] == "unlimited" )
         ulimitJson[key] = -1;
      ulimitJson[key] = parseInt( ulimitJson[key] );
   }
   return ulimitJson;
}

function getUlimitItem( cmd, PID, itemName )
{
   // limits file format is like bellow:
   // Limit                     Soft Limit           Hard Limit           Units     
   // Max cpu time              unlimited            unlimited            seconds   
   // Max file size             unlimited            unlimited            bytes     
   // Max data size             unlimited            unlimited            bytes     
   //
   // I need to get the 'Soft Limit' of 'Limit'
   var result = cmd.run( "cat /proc/" + PID + "/limits | " + 
         "grep '" + itemName + "' | " + 
         "awk -F '  +' '{print $2}'" ); 
   result = result.slice( 0, result.length - 1 );
   return result;
}

function getUlimitBySnap( db, GID, NID )
{
   var cond = {};
   if( GID != '-' ) // '-' means standalone
   {
      GID = parseInt( GID );
      NID = parseInt( NID );
      cond.NodeID = [ GID, NID ];
   }

   var cur = db.snapshot( SDB_SNAP_HEALTH, cond );
   var result = cur.next().toObj().Ulimit;
   cur.close();
   return result;
}
