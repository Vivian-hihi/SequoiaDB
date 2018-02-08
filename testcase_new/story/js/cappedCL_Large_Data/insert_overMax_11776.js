/************************************
*@Description:insert data over Max
*@author:      zhaoyu
*@createdate:  2017.7.15
*@testlinkCase: seqDB-11776
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11776";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11776";
   var insertNum = 10000;
   var clOption = {Capped:true, Size:1024, Max:insertNum, AutoIndexId:false, OverWrite:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   var expectNum = 0;
   var min = 0;
   var max = insertNum -1;
   var repeatNum = 30;
   for(var j = 0 ; j< repeatNum; j++)
   {
      var rd = new commDataGenerator();
      var recs = rd.getRecords( insertNum, [ "int", "string", "bool", "date", 
                                             "binary", "regex", "null" ],['a'] );
      insertDatas( dbcl, recs );
      println("--insert data success!");
      expectNum = expectNum + insertNum;
      
      try
      {
         dbcl.insert({b:"a"});
         throw "NEED_ERROR";
      }catch(e)
      {
         if(e !== -307)
         {
           throw buildException("insert data up to limit)", e, null, null, e);
         }
      }
      println("--insert data up to limit!");
      
      var range = max - min;
      var skipNum = Math.ceil( min + Math.random() * range );
      println("skipNum:" + skipNum);
      var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
      println("--get logicalID success!" + "logicalID:" + logicalID[0]);
      
      //随机设置pop方向
      if(skipNum % 2 === 0)
      {
         direction= -1;
      }else
      {
         direction= 1;
      }
      
      //执行pop
      pop( dbcl, logicalID[0], direction );
      
      //比较count结果
      if(direction == -1)
      {
         expectNum = skipNum;
      }else
      {
         expectNum = expectNum - skipNum - 1;
      }
      checkCount( dbcl, null, expectNum);
      println("--count success!expectCount: " + expectNum);
      
      //比较find结果
      try
      {
         dbcl.find().sort({_id:1}).limit(1);
         dbcl.find().sort({_id:1}).limit(1).skip(expectNum-1);
      }catch(e)
      {
         throw buildException("find data 1", e, null, null, e);
      }
      println("--find data success!");
      
      insertNum = max + 1 - expectNum;
      
      println("insertNum:" + insertNum);
   }
   println("check primary data success");
   
   //校验主备数据一致
   checkConsistency(db, csName, clName);
   
   //最终主备数据一致
   db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "s" } );
   dbcl = db1.getCS(csName).getCL(clName);
   
   //比较count结果
   actualNum = dbcl.count();
   if(parseInt(actualNum) !== expectNum)
   {
      println("--slave node count failed!actualNum:" + actualNum + ",expectNum: " + expectNum);
      throw "SECOND_COUNT_ERR";
   }
   
   //比较find结果
   try
   {
      dbcl.find().sort({_id:1}).limit(1);
      dbcl.find().sort({_id:1}).limit(1).skip(expectNum-1);
   }catch(e)
   {
      throw buildException("find data", e, null, null, e);
   }
   println("--slave node find data success!");
   
   commDropCS( db, csName, true, "drop CS in the end" );
   db1.close();
}
main();