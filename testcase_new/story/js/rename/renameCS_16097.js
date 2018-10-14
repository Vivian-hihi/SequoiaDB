/************************************
*@Description: 多次修改cs名
*@author:      luweikang
*@createdate:  2018.10.12
*@testlinkCase:seqDB-16097
**************************************/

main();

function main()
{
   //@ clean before
   if( true == commIsStandalone( db ) )
   {
      println( "run mode is standalone" );
      return;
   }     
    //less two groups no split
   var allGroupName = getGroupName(db,true);         
   if( 1 === allGroupName.length )
   {
      println("--least two groups");
      return ;
   }
   
   println("---begin rename cs test---");
   var oldcsName = COMMCSNAME+"_16097_old";
   var newcsName = COMMCSNAME+"_16097_new";
   var clName = CHANGEDPREFIX + "_16097_maincl";
   var subclName1 = COMMCSNAME+"_16097_sub1";
   var subclName2 = COMMCSNAME+"_16097_sub2";
   
   var cs = commCreateCS( db, oldcsName, false, "create cs in begine", "");
   var cl = commCreateCLByOption( db, oldcsName, clName, {ShardingKey:{a:1}, ShardingType:"range", ReplSize:0, Compressed: true, IsMainCL:true}, false, false, "create CL in the begin");
   var subcl1 = commCreateCLByOption( db, oldcsName, subclName1, { ShardingKey:{no: 1}, ShardingType:"range"}, false, false, "create CL in the begin");
   var subcl2 = commCreateCLByOption( db, oldcsName, subclName2, { ShardingKey:{no:1}, ShardingType:"hash" }, false, false, "create CL in the begin");
   cl.attachCL(oldcsName+"."+subclName1, { LowBound:{a:0}, UpBound:{a:1000}});
   cl.attachCL(oldcsName+"."+subclName2, { LowBound:{a:1000}, UpBound:{a:2000}});
   
   insertData(cl, 2000);
   
   println("---test rename cs---");
   for(i=0; i<10; i++)
   {
      db.renameCS(oldcsName, newcsName);
      cl = db.getCS(newcsName).getCL(clName);
      cl.update({$set:{c:"test"}});
      db.renameCS(newcsName, oldcsName);
      cl = db.getCS(oldcsName).getCL(clName);
      cl.update({$set:{c:"test"}});
   }
   
   db.renameCS(oldcsName, newcsName);
   checkRenameCSResult(oldcsName, newcsName, 2);
   
   commDropCS( db, newcsName, true, false, "clean cs---" );
   println("---end the test---");
}

