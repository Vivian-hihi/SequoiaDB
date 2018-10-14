/************************************
*@Description: 修改cs名后，检查快照、文件
*@author:      luweikang
*@createdate:  2018.10.12
*@testlinkCase:seqDB-16096
**************************************/

main();

function main()
{
   println("---begin rename cs test---");
   var oldcsName = COMMCSNAME+"_16096_old";
   var newcsName = COMMCSNAME+"_16096_new";
   var clName = CHANGEDPREFIX + "_16096_cl";
   var lobName = CHANGEDPREFIX+"_16096_lob";
   
   var cs = commCreateCS( db, oldcsName, false, "create cs in begine", "");
   var cl = commCreateCLByOption( db, oldcsName, clName, {}, false, false, "create CL in the begin");
   
   cl.createIndex( "index16096", {age: 1}, false );

   insertData(cl);
   
   var cmd5 = createFile(lobName)
   
   var lobArray = putLobs(cl, lobName)
   
   println("---test rename cs---");
   db.renameCS(oldcsName, newcsName);
   
   checkRenameCSResult(oldcsName, newcsName, 1);
   
   cl = db.getCS(newcsName).getCL(clName);
   checkLob(cl, lobArray, cmd5);
   
   commDropCS( db, newcsName, true, false, "clean cs---" );
   println("---end the test---");
}

