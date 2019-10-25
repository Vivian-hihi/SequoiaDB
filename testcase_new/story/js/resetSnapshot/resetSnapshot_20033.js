/******************************************************************************
*@Description : seqDB-20031:指定Type，Collection，重置普通集合的集合快照信息
*@author      : Zhao xiaoni
*@Date        : 2019-10-24
******************************************************************************/
try
{
   main();
}
catch(e)
{
   if(e.constructor === Error)
   {
      println(e.stack);
   }
   throw new Error(e);
}

function main()
{
   if(commIsStandalone( db ))
   {
      println("Environment is standalone");
      return;
   }
   
   var maincsName = "maincs_20033";
   var mainclName = "maincl_20033";
   var subcsName1 = "subcs1_20033";
   var subclName1 = "subcl1_20033";
   var subcsName2 = "subcs2_20033";
   var subclName2 = "subcl2_20033";
   var indexName = "index_20033";
   var indexDef = {a: 1};
   var subFullName = [subcsName1 + "." + subclName1, subcsName2 + "." + subclName2];
 
   commDropCS( db, maincsName );
   commDropCS( db, subcsName1 );
   commDropCS( db, subcsName2 );
   var maincl = db.createCS(maincsName).createCL(mainclName, {IsMainCL: true, ShardingKey: {a: 1}, ShardingType: "range", ReplSize: -1});
   var subcl1 = db.createCS(subcsName1).createCL(subclName1, {ShardingKey: {a: 1}, ShardingType: "range", Partition: 1024, ReplSize: -1});
   var subcl2 = db.createCS(subcsName2).createCL(subclName2, {ShardingKey: {a: 1}, ShardingType: "range", Partition: 1024, ReplSize: -1});
   for(var i = 0; i < subFullName.length; i++)
   {
      maincl.attachCL(subFullName[i], {LowBound:{a: i*50}, UpBound: {a: (i+1)*50}});
   }

   for(var i = 0; i < 100; i++)
   {
      maincl.insert({a: i, b: i});
      maincl.update({"$set": {b: (100 + i)}}, {a: i});
      maincl.remove({b: (100 + i)});
      var cursor = maincl.find().hint({"": "a"});
      while(cursor.next()){}
      cursor = maincl.find().hint({"": null});
      while(cursor.next()){}
   }
 
   db.resetSnapshot({Type: "collections", Collection: maincsName + "." + mainclName});
   
   var expResult = {"TotalDataRead":0,"TotalIndexRead":0,"TotalDataWrite":0,"TotalIndexWrite":0,"TotalUpdate":0,"TotalDelete":0,"TotalInsert":0,"TotalSelect":0,"TotalRead":0,"TotalWrite":0,"TotalTbScan":0,"TotalIxScan":0};
   
   for(var i = 0; i < subFullName.length; i++)
   {
      checkResult(db, subFullName[i], expResult);
   }
   
   commDropCS( db, maincsName, false);   
   commDropCS( db, subcsName1, false);
   commDropCS( db, subcsName2, false);
}
