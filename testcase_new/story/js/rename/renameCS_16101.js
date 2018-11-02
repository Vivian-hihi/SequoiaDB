/************************************
*@Description: 修改cs名后，执行数据增删改查操作--//review1:描述和实际用例步骤不符
*@author:      luweikang
*@createdate:  2018.10.12
*@testlinkCase:seqDB-16101
**************************************/

main();

function main()
{
   println("---begin rename cs test---");
   var oldcsName = COMMCSNAME+"_16101_old";
   var newcsName = COMMCSNAME+"_16101_new";
   var clName1 = CHANGEDPREFIX + "_16101_cl1";
   var clName2 = CHANGEDPREFIX + "_16101_cl2";
   
   var cs = commCreateCS( db, oldcsName, false, "create cs in begine", "");
   var cl = commCreateCLByOption( db, oldcsName, clName1, {}, false, false, "create CL in the begin");
   
   //insert 1000 data
   insertData(cl);
   
   db.renameCS(oldcsName, newcsName);
   
   checkRenameCSResult(oldcsName, newcsName, 1);
   
   cs = db.getCS(newcsName);
   
   //rename cs,drop cl in the end
   cs.dropCL(clName1);
   
   //rename cs,drop cl in the end
   cl = cs.createCL(clName2);
   
   //insert 1000 data, and check data
   insertData(cl, 1000);//review 2：insert没有校验结果
   
   checkRenameCSResult(oldcsName, newcsName, 1);
   
   commDropCS( db, newcsName, true, false, "clean cs---" );
   println("---end the test---");
}
