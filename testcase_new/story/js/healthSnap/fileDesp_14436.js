/*******************************************************************
* @Description : test health snapshot field "FileDesp"
*                seqDB-14436:FileDesp字段测试
* @author      : linsuqiang
* 
*******************************************************************/

main( db );

function main( db )
{
   var cmd = new Cmd();
   var nodeInfo = getLocalNodeInfo( cmd );
   var expFileDesp = getFileDespByCmd( cmd, nodeInfo.PID );
   var actFileDesp = getFileDespBySnap( db, nodeInfo.GID, nodeInfo.NID );
   assert( isEquals( expFileDesp, actFileDesp ), "wrong FileDesp \n" +
         "expect " + JSON.stringify( expFileDesp ) + " \n" +
         "actual " + JSON.stringify( actFileDesp ) );
}

function getFileDespByCmd( cmd, PID )
{
   var result = cmd.run( "ls /proc/" + PID + "/fd | wc -l" );
   result = result.slice( 0, result.length - 1 );
   result = parseInt( result );
   return result;
}

function getFileDespBySnap( db, GID, NID )
{
   var cond = {};
   if( GID != '-' ) // '-' means standalone
   {
      GID = parseInt( GID );
      NID = parseInt( NID );
      cond.NodeID = [ GID, NID ];
   }

   var cur = db.snapshot( SDB_SNAP_HEALTH, cond );
   var fileDesp = cur.next().toObj().FileDesp;
   cur.close();
   // snapshot temporarily needs one FileDesp
   var usedFileDesp = fileDesp.TotalNum - fileDesp.FreeNum - 1;
   return usedFileDesp;
}
