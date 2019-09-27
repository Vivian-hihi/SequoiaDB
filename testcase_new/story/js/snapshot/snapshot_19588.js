/******************************************************************************
*@Description : seqDB-19588:检查snapshot(SDB_SNAP_SVCTASKS)快照信息
*               TestLink : seqDB-19588
*@author      : wangkexin
*@Date        : 2019-09-27
******************************************************************************/
main(db) ;
function main()
{
   var fields = ["TaskName", "TaskID", "Time", "TotalContexts", "TotalDataRead", "TotalIndexRead", "TotalDataWrite", "TotalIndexWrite", "TotalUpdate", "TotalDelete", "TotalInsert", "TotalSelect", "TotalRead", "TotalWrite"];
   var cur = db.snapshot( SDB_SNAP_SVCTASKS );
   while( cur.next() )
   {
      var ret = cur.current().toObj();
      for(var index in fields)
      {
         var field = fields[index];
         if (ret[field] == null)
         {
            println(field + ": " + ret[field]);
            throw buildException("compare field", "", "snapshot( SDB_SNAP_SVCTASKS)", "not null", "null");
         }
      }
   }
}