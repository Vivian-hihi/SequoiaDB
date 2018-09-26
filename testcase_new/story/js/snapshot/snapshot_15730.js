/******************************************************************************
*@Description : test SdbSnapshotOption创建存储过程
*               TestLink : seqDB-15730:使用SdbSnapshotOption创建存储过程
*@auhor       : CSQ 
******************************************************************************/

function main()
{
   if (commGetGroupsNum(db) < 1)
   {
      return ;
   }
   try{
      commDropCS( db, COMMCSNAME+"15730", true, "drop CS "+COMMCSNAME+"15730" );
   }catch( e ){}
   var varCS = commCreateCS( db, COMMCSNAME+"15730", true, "create CS" );
   var groups = commGetGroups(db);
   var srcGroupName = groups[0][0].GroupName;
   var varCL = varCS.createCL(COMMCLNAME+"15730",{ShardingKey:{a:1},ShardingType:"hash",Group:srcGroupName});
   var clfullname = COMMCSNAME+"15730."+COMMCLNAME+"15730";
   db.createProcedure(function test15730(clfullname){ return new SdbSnapshotOption().cond({Name:clfullname}).sel({Name:1,UniqueID:1}).sort({Version:1}).options({expand:true}).limit(1).skip(0).flags(1);});
   
   var a = db.eval( 'test15730("'+clfullname+'")' );
   var cur = db.snapshot(SDB_SNAP_CATALOG, a);
   var size=0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();
      if( ret.toObj().Name !== clfullname)
      {
         throw buildException("check count", "", "test SdbSnapshotOption创建存储过程error!", clfullname, ret.toObj().Name);
      }
   }
   if( size <= 0 )
   {
      throw buildException("check count", "", "snapshot(SDB_SNAP_CONFIGS,new SdbSnapshotOption().options({\"expand\":false}))", ">0", "<=");
   }
   try{
      commDropCS( db, COMMCSNAME+"15730", true, "drop CS "+COMMCSNAME+"15730" );
      db.removeProcedure("test15730");
   }
   catch( e )
   {
      throw buildException("teardown 15730 fail", e, "clear", "success", e);
   }
}

main(db) ;