/******************************************************************************
*@Description : seqDB-20032:指定Type，CollectionSpace，重置普通集合的集合快照信息
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
   
   var csName = "cs_20032";
   var clName = "cl_20032";
   var replSize = -1;
   var fullName = csName + "." + clName;
   var indexName = "index_20032";
   var indexDef = {a: 1};
   
   commDropCL( db, csName, clName );
   var cl = commCreateCL( db, csName, clName, replSize );
   commCreateIndex( cl, indexName, indexDef, false );

   for(var i = 0; i < 100; i++)
   {
      cl.insert([{a: i}, {b: i}]);
      cl.update({"$set": {a: (100 + i)}}, {a: i});
      cl.remove({a: (100 + i)});
      var cursor = cl.find().hint({"": "a"});
      while(cursor.next()){}
      cursor = cl.find().hint({"": null});
      while(cursor.next()){}
   }
 
   db.resetSnapshot({Type: "collections", CollectionSpace: csName});
   
   var expResult = {"TotalDataRead":0,"TotalIndexRead":0,"TotalDataWrite":0,"TotalIndexWrite":0,"TotalUpdate":0,"TotalDelete":0,"TotalInsert":0,"TotalSelect":0,"TotalRead":0,"TotalWrite":0,"TotalTbScan":0,"TotalIxScan":0};
   checkResult(db, fullName, expResult);
   
   commDropCL( db, csName, clName, false, false );   
}
