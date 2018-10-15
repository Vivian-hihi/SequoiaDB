/************************************
*@Description: 事务操作过程中修改cs名
*@author:      luweikang
*@createdate:  2018.10.12
*@testlinkCase:seqDB-16105
**************************************/

main();

function main()
{
   println("---begin rename cs test---");
   var oldcsName = COMMCSNAME+"_16105_old";
   var newcsName = COMMCSNAME+"_16105_new";
   var clName = CHANGEDPREFIX + "_16105_cl";
   
   var cs = commCreateCS( db, oldcsName, false, "create cs in begine", "");
   var cl = commCreateCLByOption( db, oldcsName, clName, {}, false, false, "create CL in the begin");
   
   db.transBegin();
   cl.insert({"no":10086, customerName:"testTrans", "phone":13700010086, "openDate":1402990912105});
   cl.insert({"no":10000, customerName:"testTrans", "phone":13700010000, "openDate":1402990912106});
   
   try
   {
      db.renameCS(oldcsName, newcsName);
   }
   catch(e)
   {
      if(e!==-3)
      {
         throw buildException("rename cs during the trans", e, "trans", -3, e);
      }
   }
   
   db.renameCS(oldcsName, newcsName);
   
   cl = db.getCS(newcsName).getCL(clName);
   checkCLRecord(cl, {customerName: 'testTrans'}, 2);
   
   checkRenameCSResult(newcsName, oldcsName, 1);
   
   commDropCS( db, oldcsName, true, false, "clean cs---" );
   println("---end the test---");
}

function checkCLRecord( cl, cond, expNum )
{
   var actNum = cl.count(cond);
   if(actNum != expNum){
      throw buildException("checkCLRecord", "", "check the cl recond num error", expNum, actNum);
   }
}
