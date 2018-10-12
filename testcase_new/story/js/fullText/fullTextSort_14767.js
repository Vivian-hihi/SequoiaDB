/************************************
*@Description: full text sort
*@author:      zhaoyu
*@createdate:  2018.10.12
**************************************/
function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Deploy mode is standalone!" );
      return;
   }
   
   var clName = COMMCLNAME + "_ES_14767";
   var clFullName = COMMCSNAME + "." + clName
   var indexName = "a";
   
   commDropCL( db, COMMCSNAME, clName);
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, indexName, {a:"text"});
   dbcl.insert({a:"text"});
   
   var esOperator = new ESOperator();
   var dbOperator = new DBOperator();
   var eSIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, indexName);
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   
   //not support full text sort
   try
   {
      var cursor = dbcl.find({"":{$Text:{query:{match_all:{}},sort:[{a:{order:"desc"}}]}}});
      while(cursor.next()){}
      throw "NEED_SORT_ERR";
   }catch(e)
   {
      if(e !== -6)
      {
         throw e;
      }
   }
   
   commDropCL( db, COMMCSNAME, clName);
}
main()