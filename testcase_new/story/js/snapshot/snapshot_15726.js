/******************************************************************************
*@Description : test snapshot SDB_SNAP_CONFIGS 
*               TestLink : seqDB-15726:指定快照查询参数查询快照信息（一个参数）
*@auhor       : CSQ 
******************************************************************************/
/*
 1、指定快照查询参数查询快照信息，可随机指定一种快照类型，快照参数测试如下： cond：指定一个条件、多个条件、指定条件匹配不到记录 sel：指定多个字段名、指定字段名中包含不存在的字段名、指定字段名不存在 sort：指定降序、升序 skip：指定起始记录、最后一条记录、超过记录数 limit：指定一条、所有记录、超过记录数 options：指定js对象，如指定flag值、配置快照参数 3、检查快照返回结果集 
*/
function main()
{
   if (commGetGroupsNum(db)<2)
   {
      return ;
   }
   //cond：指定一个条件
   var cur = db.snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({role:"catalog"}));
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().role != "catalog" )
      {
         throw buildException("check record", "", "snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({role:\"catalog\"}))", "catalog", ret.toObj().role);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({role:\"catalog\"}))", ">0", "<=0");
   }
   //cond多个条件
   cur = db.snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}));
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().IsPrimary != true )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}))", true, ret.toObj().IsPrimary);
      }
      if( ret.toObj().ServiceStatus != true )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}))", true, ret.toObj().ServiceStatus);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check size", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().cond({$and:[{IsPrimary:true}, {ServiceStatus:true}]}))", ">0", "<=0");
   }
   //指定条件匹配不到记录 
   cur = db.snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({IsPrimary:true}));
   var size=0;
   while( cur.next() )
   {
      size++;  
   } 
   if( size !== 0 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().cond({IsPrimary:true}));", 0, size);
   }
   //sel指定多个字段名
   cur = db.snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}));
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().IsPrimary == undefined )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}))", "not equals undefined", ret.toObj().IsPrimary);
      }
      if( ret.toObj().ServiceStatus == undefined )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}))", "not equals undefined", ret.toObj().ServiceStatus);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}))", ">0", "<=0");
   }
   //sel指定字段名中包含不存在的字段名
   cur = db.snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({"csq":{$include:1},"IsPrimary":{$include:1}}));
   var size = 0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().csq !== undefined )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}))", "undefined", ret.toObj().csq);
      }
      if( ret.toObj().IsPrimary == undefined )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}))", "not equals undefined", ret.toObj().IsPrimary);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}))", ">0", "<=0");
   }
   //sel指定字段名不存在
   cur = db.snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({"csq":{$include:1}}));
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toString() != "{}" )
      {
         throw buildException("check record", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}));", "{}", ret.toString());
      }
   }
   if( size <= 0 )
   {
     throw buildException("check count", "", "snapshot(SDB_SNAP_HEALTH,new SdbSnapshotOption().sel({IsPrimary:1, ServiceStatus:1}));", ">0", "<=0");
   }
   //sort指定降序
   cur = db.snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().sort({svcname:-1}));
   var size=0;
   var tmp = 66660;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if(ret.toObj().svcname > tmp)
      {
         throw buildException("check sort ", "SDB_SNAP_CONFIGS,new SdbSnapshotOption().sort({svcname:-1}))", "<=", ">");
      }
      tmp = ret.toObj().svcname;
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().sort({svcname:-1}))", ">0", "<=0");
   }
   //sort升序 
   cur = db.snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().sort({svcname:1}));
   var size=0;
   var tmp = 0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if(ret.toObj().svcname < tmp)
      {
         throw buildException("check sort ", " ", "snapshot( SDB_SNAP_CONFIGS,new SdbSnapshotOption().sort({svcname:1}))", ">=", "<");
      }
      tmp = ret.toObj().svcname;
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().sort({svcname:1}))", ">0", "<=0");
   }
   //skip：指定起始记录、最后一条记录、超过记录数 limit：指定一条、所有记录、超过记录数 
   cur = db.snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().skip(0) );
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 1 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().skip(0))", 1, size);
   }
   cur = db.snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().skip(1) );
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 0 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().skip(1) )", 0, size);
   }
   cur = db.snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().skip(2) );
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 0 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().skip(2) )", 0, size);
   }
   cur = db.snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().limit(0) );
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 0 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().limit(0) )", 0, size);
   }
   cur = db.snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().limit(1) );
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 1 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().limit(1) )", 1, size);
   }
   cur = db.snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().limit(2) );
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
   }
   if( size !== 1 )
   {
      throw buildException("check count", "", "snapshot( SDB_SNAP_SYSTEM, new SdbSnapshotOption().limit(2) )", 1, size);
   }
   //options：指定js对象，如指定flag值、配置快照参数
   cur = db.snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().options({"expand":false}));
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().archiveon !== undefined)
      {
         throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().options({\"expand\":false}))", undefined, ret.toObj().archiveon);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().options({\"expand\":false}))", ">", "<=");
   }
}

//main(db) ;