/************************************
*@Description:repeat insert/pop/find
*@author:      zhaoyu
*@createdate:  2017.7.14
*@testlinkCase: seqDB-11779
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11779";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11779";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   //插入500万条记录，数据长度随机，使数据文件被扩展一次
   var insertTimes = 10;
   for(var i =0; i < insertTimes; i++)
   {
      var rd = new commDataGenerator();
      var recordNum = 10000;
      var recs = rd.getRecords( recordNum, [ "int", "string", "bool", "date", 
                                             "binary", "regex", "null" ],['a'] );
      insertDatas( dbcl, recs );
   }
   println("--insert data success!");
   
   var expectNum = recordNum * insertTimes;
   var direction =1;
   
   //循环pop、查询、插入，logicaID随机
   var repeatNum = 10;
   for(var j = 0 ; j< repeatNum; j++)
   {
      //随机获取某条记录的logicalID
      var min = 0;
      var max = repeatNum * recordNum;
      var range = max - min;
      var skipNum = Math.ceil( min + Math.random() * range );
      var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
      println("--get logicalID success!");
      
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
      println("--count success! expectNum: " + expectNum);
      
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
      
      //再次insert 500万条记录
      for(var i =0; i < insertTimes; i++)
      {
         var rd = new commDataGenerator();
         var recs = rd.getRecords( recordNum, [ "int", "string", "bool", "date", 
                                                "binary", "regex", "null" ],['a'] );
         insertDatas( dbcl, recs );
      }
      
      println("--insert data afer pop success!");
      
      expectNum = expectNum + max;
      max = expectNum;
      
      //比较count结果
      checkCount( dbcl, null, expectNum);
      println("--count success after pop! expectNum: " + expectNum);
      
      //比较find结果
      try
      {
         dbcl.find().sort({_id:1}).limit(1);
         dbcl.find().sort({_id:1}).limit(1).skip(expectNum-1);
      }catch(e)
      {
         throw buildException("find data 2", e, null, null, e);
      }
      println("--find data after pop success!repeatNum : " + j);
   }
   
   commDropCS( db, csName, true, "drop CS in the end" );
   
}

main();