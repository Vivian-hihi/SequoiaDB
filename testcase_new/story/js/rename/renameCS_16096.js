/************************************
*@Description: 修改cs名后，检查快照、文件
*@author:      luweikang
*@createdate:  2018.10.12
*@testlinkCase:seqDB-16096
**************************************/

main();

function main()
{
   println("---begin rename cs test---");//1、这个println和下面一个println是重复的，建议保留下面的，或者描述实现的执行步骤
   var oldcsName = COMMCSNAME+"_16096_old";
   var newcsName = COMMCSNAME+"_16096_new";
   var clName = CHANGEDPREFIX + "_16096_cl";
   var lobName = CHANGEDPREFIX+"_16096_lob";
   
   var cs = commCreateCS( db, oldcsName, false, "create cs in begine", "");
   var cl = commCreateCLByOption( db, oldcsName, clName, {}, false, false, "create CL in the begin");
   
   cl.createIndex( "index16096", {age: 1}, false );//2、这步操作后面没有做检查

   insertData(cl);//3，同问题2，修改cs前的操作，修改cs后建议做结果检测，这个文本用例也需要补充测试点
   
   var cmd5 = createFile(lobName);
   
   var lobArray = putLobs(cl, lobName);
   
   println("---test rename cs---");
   db.renameCS(oldcsName, newcsName);
   
   checkRenameCSResult(oldcsName, newcsName, 1);
   
   cl = db.getCS(newcsName).getCL(clName);
   checkLob(cl, lobArray, cmd5);
   
   commDropCS( db, newcsName, true, false, "clean cs---" );
   println("---end the test---");//2、用例起始和结束这种可以不需要打印处理，println建议打印主要的用例执行步骤
}

