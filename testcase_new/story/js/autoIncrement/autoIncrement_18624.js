/************************************
*@Description: seqDB-18624:反转自增队列方向，自增字段已使用时，使CurrentValue不在修改后的区间内
*@Author     : 2019.07.24 yinzhen 
**************************************/
main();

function main(){
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
	  return;
   }
   
   var clName = COMMCLNAME + "_auto_increment_18624";
   commDropCL( db, COMMCSNAME, clName, true, true ); 
   var cl = commCreateCL( db, COMMCSNAME, clName );
   
   cl.createAutoIncrement({Field: "id1", CacheSize:1, AcquireSize:1, Increment:-1, StartValue:-100});
   cl.createAutoIncrement({Field: "id2", CacheSize:1, AcquireSize:1, Increment:1, StartValue:100});

   // 检查自增字段属性是否正确
   var clUniqueID = getCLID(COMMCSNAME, clName);
   var sequenceName_1 = "SYS_" + clUniqueID + "_id1_SEQ";
   var sequenceName_2 = "SYS_" + clUniqueID + "_id2_SEQ";
   var expSequenceObj_1 = {CacheSize:1, AcquireSize:1, Increment:-1, StartValue:-100, CurrentValue:-100, "MaxValue":-1, "MinValue":{"$numberLong":"-9223372036854775808"}};
   var expSequenceObj_2 = {CacheSize:1, AcquireSize:1, Increment:1, StartValue:100, CurrentValue:100};
   checkSequence(sequenceName_1, expSequenceObj_1);
   checkSequence(sequenceName_2, expSequenceObj_2);
   
   // 通过本coord和其它coord插入记录查询
   var coordList = getCoordNodeNames();
   var countFlag = {count:0};
   var expList = [];
   for(var i in coordList){
	   var dbcl = new Sdb(coordList[i]).getCS(COMMCSNAME).getCL(clName);
	   var cur = dbcl.find().sort({"id1":1});
	   insertData_1(dbcl, countFlag, expList);
	   checkRec( cur, expList );
   }
   
   // 修改自增属性字段，自增序列修改为自减序列、自减序列修改为自增序列，同时使修改前自增序列的CurrentValue不在修改后[MinValue,MaxValue]范围内
   expSequenceObj_1 = {CacheSize:1, AcquireSize:1, Increment:1, StartValue:102, CurrentValue:(-100 - countFlag.count + 1), "MaxValue":100000, "MinValue":0};
   expSequenceObj_2 = {CacheSize:1, AcquireSize:1, Increment:-1, StartValue:-202, CurrentValue:(100 + countFlag.count - 1), MinValue:-100000, MaxValue:-200};
   cl.alter({AutoIncrement:{Field: "id1", CacheSize:1, AcquireSize:1, Increment:1, StartValue:102, MinValue:0, MaxValue:100000}});
   cl.alter({AutoIncrement:{Field: "id2", CacheSize:1, AcquireSize:1, Increment:-1, StartValue:-202, MinValue:-100000, MaxValue:-200}});
   checkSequence(sequenceName_1, expSequenceObj_1);
   checkSequence(sequenceName_2, expSequenceObj_2);
   
   // 通过本coord和其它coord插入记录查询，插入记录报错-325
   try{
      for(var i in coordList){
         var dbcl = new Sdb(coordList[i]).getCS(COMMCSNAME).getCL(clName);
         var cur = dbcl.find().sort({"id1":1});
         insertData_1(dbcl, countFlag, expList);
         checkRec( cur, expList );
      }
   }catch(e){
      if(-325 !== e){
         throw "INSERT ERROR EXPECT -325";
      }
   }
   
   // 继续修改currentValue，在[MinValue,MaxValue)范围内
   expSequenceObj_1 = {CacheSize:1, AcquireSize:1, Increment:1, StartValue:102, CurrentValue:110, "MaxValue":100000, "MinValue":0};
   expSequenceObj_2 = {CacheSize:1, AcquireSize:1, Increment:-1, StartValue:-202, CurrentValue:-220, MinValue:-100000, MaxValue:-200};
   cl.alter({AutoIncrement:{Field: "id1", CurrentValue: 110}});
   cl.alter({AutoIncrement:{Field: "id2", CurrentValue: -220}});
   checkSequence(sequenceName_1, expSequenceObj_1);
   checkSequence(sequenceName_2, expSequenceObj_2);
   
   // 通过本coord和其它coord插入记录查询
   var count = countFlag.count - 1;
   for(var i in coordList){
	  println(coordList[i]);
      var dbcl = new Sdb(coordList[i]).getCS(COMMCSNAME).getCL(clName);
      var cur = dbcl.find().sort({"id1":1});
      insertData_2(dbcl, countFlag, count, expList);
      checkRec( cur, expList );
   }
   
   commDropCL( db, COMMCSNAME, clName, true, true );
}

function insertData_1(cl, countFlag, expList)
{
	for(var i = countFlag.count; i < countFlag.count + 3; i++){
		cl.insert({a:i});
		expList.push({a:i, id1:-100 - i, id2: 100 + i});
	}
	countFlag.count = countFlag.count + 3;
	expList.sort(compare("id1"))
}

function insertData_2(cl, countFlag, count, expList)
{
	for(var i = countFlag.count; i < countFlag.count + 3; i++){
		cl.insert({a:i});
		expList.push({a:i, id1:110 + i - count, id2: -220 - i + count});
	}
	countFlag.count = countFlag.count + 3;
	println(countFlag.count);
	expList.sort(compare("id1"))
}