/************************************
*@Description: 事务操作过程中修改cs1名
*@author:      luweikang
*@createdate:  2018.10.12
*@testlinkCase:seqDB-16106
**************************************/

main();

function main()
{
   println("---begin rename cs1 test---");
   var csName1 = COMMCSNAME+"_16106_1";
   var csName2 = COMMCSNAME+"_16106_2";
   var newcs1Name = COMMCSNAME+"_16106_new";
   var clName1 = CHANGEDPREFIX + "_16106_cl11";
   var clName2 = CHANGEDPREFIX + "_16106_cl12";
   
   var cs1 = commCreateCS( db, csName1, false, "create cs1 in begine", "");
   var cs2 = commCreateCS( db, csName2, false, "create cs1 in begine", "");
   var cl1 = commCreateCLByOption( db, csName1, clName1, {}, false, false, "create cl1 in the begin");
   var cl2 = commCreateCLByOption( db, csName2, clName2, {}, false, false, "create cl1 in the begin");
   
   println('---trans begin---');
   db.transBegin();
   cl1.insert({"no":10086, customerName:"testTrans", "phone":13700010086, "openDate":1402990912105});
   cl1.insert({"no":10000, customerName:"testTrans", "phone":13700010000, "openDate":1402990912106});
   
   // rename no trans cs
   println('---rename cs---');
   db.renameCS(csName2, newcs1Name);
   
   db.transCommit();
   println('---trans commit---');
   
   cl2 = db.getCS(newcs1Name).getCL(clName2);
   checkcl1Record(cl1, {customerName: 'testTrans'}, 2);
   
   checkRenameCSResult(csName2, newcs1Name, 1);
   
   commDropCS( db, csName1, true, false, "clean cs1---" );
   commDropCS( db, newcs1Name, true, false, "clean cs2---" );
   println("---end the test---");
}

function checkcl1Record( cl1, cond, expNum )
{
   var actNum = cl1.count(cond);
   if(actNum != expNum){
      throw buildException("checkcl1Record", "", "check the cl1 recond num error", expNum, actNum);
   }
}
