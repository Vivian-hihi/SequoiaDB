/******************************************************************************
*@Description : test snapshot SDB_SNAP_HEALTH 
*               TestLink : seqDB-15725:指定快照查询参数查询节点健康检测快照
*@auhor       : CSQ 
******************************************************************************/

function main()
{
   var cur;
   try
   {
      cur = db.snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}).sel({IsPrimary:1}).limit(1));
   }
   catch( e )
   {
      throw buildException("seqDB-15725 test snapshot fail", e, "test snapshot", "success", e);
   }
   var size=0;
   var expected = "{\"IsPrimary\": true}";
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().IsPrimary != true )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}).sel({IsPrimary:1}).limit(1))", true, ret.toObj().IsPrimary);
      }
   }
   if( size !== 1 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}).sel({IsPrimary:1}).limit(1))", 1, size);
   }
}

main(db) ;