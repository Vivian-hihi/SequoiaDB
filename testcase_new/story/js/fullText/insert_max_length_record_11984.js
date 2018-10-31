/***************************************************************************
@Description :seqDB-11984 :记录中字段长度最大(略<16M)，创建全文索引 
@Modify list :
              2018-10-25  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_11984";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   var a = new Array(1024*1024*16 - 5*1024).join("a");
   dbcl.insert({a : a});
   dbcl.createIndex( "a", {a : "text"});
   
   checkFullSyncToES(COMMCSNAME, clName, "a", 1);
   
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL( dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {a : ""});
   var expResult = dbOperator.findFromCL( dbcl, null, {a : ""});
     
   if (expResult.length != 1){
      throw buildException("main()", "unexpect records", "equal", 1, expResult.length);
   }
   if (expResult[0]["a"] != actResult[0]["a"]){
      throw buildException("main()", "unexpect record value", "equal", expResult[0]["a"], actResult[0]["a"]);
   }
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()