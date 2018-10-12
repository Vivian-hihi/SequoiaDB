/************************************
*@Description: insert record with 16M
*@author:      zhaoyu
*@createdate:  2018.9.28
**************************************/
function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Deploy mode is standalone!" );
      return;
   }
   
   var clName = COMMCLNAME + "_ES_14371";
   var clFullName = COMMCSNAME + "." + clName
   var indexName = "a";
   
   commDropCL( db, COMMCSNAME, clName);
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, indexName, {a:"text"});
   
   var str = new Array(1024*1024*15).join("a");
   dbcl.insert({a:str});
   
   //check count,but not check record(out of memery)
   var esOperator = new ESOperator();
   var dbOperator = new DBOperator();
   var eSIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, indexName);
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   println("---check insert success---");
   
   var str = new Array(1024*1024*16).join("a");
   try
   {
      dbcl.insert({a:str});
      throw "NEED_INSERT_ERR";
   }catch(e)
   {
      if(e !== -24)
      {
         throw e;
      }
   }
   
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   
   commDropCL( db, COMMCSNAME, clName);
}
main()