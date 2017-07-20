/************************************
*@Description:insert data over Max
*@author:      zhaoyu
*@createdate:  2017.7.15
*@testlinkCase: seqDB-12140
**************************************/
function main()
{
   var csName = COMMCSNAME + "_12140";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_12140";
   var clOption = {Capped:true, Size:33554432, AutoIndexId:false, OverWrite:true};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   db.setSessionAttr( { PreferedInstance: "M" } );
   
   var insertNum = 32768;
   var firstBlockRecordNum = 0; 
   var expectLogicalID = 0;
   var min = 0;
   var max = insertNum -1;
   var repeatNum = 10;
   var stringLength = 969;
   for(var j = 0 ; j< repeatNum; j++)
   {
      //插入刚好Size记录
      insertFixedLengthDatas( dbcl, insertNum, stringLength, "a" );
      println("--insert data success! insertNum: " + insertNum);
      
      //获取第1条及最后一条记录的_id值
      var firstLogicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, null);
      var lastLogicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, max);
      var expectLogicalID = parseInt(lastLogicalID) + stringLength + 55;
      
      //再次插入1条记录并获取_id值
      var rd = new commDataGenerator();
      insertFixedLengthDatas( dbcl, 1, stringLength, "a" );
      println("--insert data up to limit!");
      lastLogicalID = getLogicalID(dbcl, null, null, {_id:-1}, 1, null);
      var expectNum = 2;
      
      //校验_id值是否正确
      if(expectLogicalID !== lastLogicalID[0])
      {
         println("expectLogicalID: " + expectLogicalID + " ,actualID: " + lastLogicalID[0]);
         throw "LOGICAL_ID_ERROR";
      }
      println("--check logical id success! logicalID: " + lastLogicalID[0]);
      
      //校验count是否正确
      checkCount( dbcl, null, expectNum);
      println("--count success!expectCount:" + expectNum);
       
      //比较find结果
      try
      {
         dbcl.find().sort({_id:1}).limit(1);
         dbcl.find().sort({_id:1}).limit(1).skip(expectNum-1);
      }catch(e)
      {
         throw buildException("find data", e, null, null, e);
      }
      println("--find data success!");
      
      //再次插入刚好填满Size的记录数
      overSizeNum = max - 1;
      insertFixedLengthDatas( dbcl, overSizeNum, stringLength, "a" );
      println("--insert data success! insertNum: " + overSizeNum);
      expectNum = max + 1;

      //随机指定LogicalID
      var range = max - min;
      var skipNum = Math.ceil( min + Math.random() * range );
      println("skipNum: " + skipNum);
      var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
      
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
         firstBlockRecordNum = max;
         println("firstBlockRecordNum: " + firstBlockRecordNum);
      }else
      {
         expectNum = expectNum - skipNum - 1;
         firstBlockRecordNum = expectNum - 1;
         println("firstBlockRecordNum: " + firstBlockRecordNum);
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
      
      //再次插入记录使超过Size空间
      insertNum = max + 1 - expectNum + 1 ;
      println("--after pop insertNum:"+ insertNum);
      insertFixedLengthDatas( dbcl, insertNum, stringLength, "a" );
      
      //比较count结果
      var expectNum = expectNum + insertNum - firstBlockRecordNum;
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
      
      insertNum = max + 1 - expectNum  ;
      
      println("repeat do " + j + " times success!");
   }
   
   //最终主备数据一致
   db.setSessionAttr( { PreferedInstance: "s" } );
   
   //比较count结果
   var flag = false;
   for(var i=0; i< 30; i++)
   {
	  sleep(1000);
	  actualNum = dbcl.count();
	  if(parseInt(actualNum) !== expectNum)
	  {
		 continue;
	  }else
	  {
		  flag = true;
		  break;
	  }
   }
   
   if(flag !== true)
   {
	   println("after 30s,the second node is not the same the primary");
	   throw "SECOND_COUNT_ERR";
   }
   println("--second node count success!actualNum:" + actualNum + ",expectNum: " + expectNum);
   
   //比较find结果
   try
   {
      dbcl.find().sort({_id:1}).limit(1);
      dbcl.find().sort({_id:1}).limit(1).skip(expectNum-1);
   }catch(e)
   {
      throw buildException("find data", e, null, null, e);
   }
   println("--second node find data success!");
   
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();