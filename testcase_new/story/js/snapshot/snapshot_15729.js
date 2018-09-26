/******************************************************************************
*@Description : test 使用SdbOptionBase查询快照信息
*               TestLink : seqDB-15729
*@auhor       : CSQ 
******************************************************************************/

function main()
{
   var option = new SdbOptionBase();
   option.cond({role:"data"}).sel({role:1,svcname:1}).sort({svcname:1}).hint({$Options:{expand:true}}).limit(5).skip(0).flags(1);
   var cur = db.snapshot( SDB_SNAP_CONFIGS, option);
   var size=0;
   var tmp = 0;
   while( cur.next() )
   {
      size++;
      var ret = cur.current();  
      if(ret.toObj().svcname < tmp)
      {
         throw buildException("compare svcname", "", "snapshot( SDB_SNAP_CONFIGS,option)", ">=", "<");
      }
      tmp = ret.toObj().svcname;
   }
   if( size <= 0 )
   {
      throw buildException("check count", e, "snapshot(SDB_SNAP_CONFIGS,option)", ">0", "<=0");
   }
}

main(db) ;