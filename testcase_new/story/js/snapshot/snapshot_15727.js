/******************************************************************************
*@Description : test snapshot SDB_SNAP_CONFIGS 
*               TestLink : seqDB-15727:指定快照查询参数查询快照信息（多个参数）
*@auhor       : CSQ 
******************************************************************************/

function main()
{
   if (commGetGroupsNum(db)<2)
   {
      return ;
   }
   var groups = commGetGroups(db);
   var groupname = groups[0][0].GroupName;
   //cond+skip+limit
   cur = db.snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:"data"}]}).sel({"archiveon":{$include:1},"role":{$include:1},"NOO":{$include:1}}).skip(commGetGroupsNum(db)-1).limit(commGetGroupsNum(db)).sort({role:1}));
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 1 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:\"data\"}]}).sel({\"archiveon\":{$include:1},\"role\":{$include:1},\"NOO\":{$include:1}}).skip(commGetGroupsNum(db)-1).limit(commGetGroupsNum(db)).sort({NOO:1}))", 1, size);
   }
   
   //所有参数组合
   cur = db.snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:"data"}]}).sel({"archiveon":{$include:1},"NOO":{$include:1}}).skip(0).limit(1).sort({NOO:1}).options({"expand":false}))
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if (ret.toString() !== "{}") 
      {
         throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:\"data\"}]}).sel({\"archiveon\":{$include:1},\"NOO\":{$include:1}}).skip(0).limit(1).sort({NOO:1}).options({\"expand\":false}))", "{}", ret.toString());
      }
   }
   if( size !== 1 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:\"data\"}]}).sel({\"archiveon\":{$include:1},\"NOO\":{$include:1}}).skip(0).limit(1).sort({NOO:1}).options({\"expand\":false}))", 1, size);
   }
   //cond+options
   cur = db.snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:"data"}]}).options({"expand":false}))
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().archiveon !== undefined)
      {
         throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({$and:[{GroupName:groupname}, {role:\"data\"}]}).options({\"expand\":false}))", undefined, ret.toObj().archiveon);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "test snapshot", ">0", "<=0");
   }
}

main(db) ;