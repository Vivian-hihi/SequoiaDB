/************************************
*@Description:seqDB-11775:扩块/扩文件时插入记录/seqDB-11792:pop多个块的数据                 
*@author:      zhaoyu
*@createdate:  2017.7.17
*@testlinkCase: seqDB-11775/seqDB-11792
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11775";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11775";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   //多次循环使扩文件
   var repeatNum = 10;
   for(var i = 0;i < repeatNum; i++)
   {
      //插入1个块的记录
      insertNum = 32768;
      stringLength = 969;
      var recordHead = 55;
      var gap = 1024 - 36;
      var expectRecords = insertFixedLengthDatas( dbcl, insertNum, stringLength, "a" );
      println("--insert data success!");
      
      //校验每次插入的第一条记录
      var expRecsFirstRecord = [];
      var expectIDs1 = [];
      var tempId1 = insertNum * i;
      var recordLength = stringLength + recordHead;
      expectIDs1.push(tempId1 * (recordLength) + i * gap);
      checkLogicalID( dbcl, null, null, {_id:1}, 1, tempId1, expectIDs1);
      println("--check first record logical ID success!");
      
      expRecsFirstRecord.push(expectRecords[0]);
      checkRecords( dbcl, null, null, {_id:1}, 1, tempId1, expRecsFirstRecord );
      println("--check first record data success!");
      
      //校验每次插入的最后一条记录
      var expRecsLastRecord = [];
      var expectIDs2 = [];
      var tempId2 = insertNum * (i + 1) -1;
      expectIDs2.push(tempId2 * (recordLength) + (i + 1) * gap);
      checkLogicalID( dbcl, null, null, {_id:1}, 1, tempId2, expectIDs2);
      println("--check last record logical ID success!");
      
      expRecsLastRecord.push(expectRecords[insertNum -1]);
      checkRecords( dbcl, null, null, {_id:1}, 1, tempId2, expRecsLastRecord );
      println("--check last record data success!");
   }
   
   //插入记录使扩展文件后，检查记录数
   var expectCount = repeatNum * insertNum;
   checkCount( dbcl, null, expectCount);
   println("--check count success!");
   
   //逆向pop单个块
   var skipNum = (insertNum -1) * (repeatNum - 1);
   var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
   pop( dbcl, logicalID[0], -1 );
   
   var expectIDs = [];
   expectIDs.push((skipNum -1) * (recordLength) + (repeatNum - 2) * gap);
   checkLogicalID( dbcl, null, null, {_id:1}, 1, skipNum - 1, expectIDs);
   println("--check last record logical ID success!");
   
   expectCount = skipNum;
   checkCount( dbcl, null, expectCount);
   println("--check count success!");
   
   //逆向pop多个块
   var skipNum = (insertNum -1) * (repeatNum - 3);
   var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
   pop( dbcl, logicalID[0], -1 );
   
   var expectIDs = [];
   expectIDs.push((skipNum -1) * recordLength + (repeatNum - 4) * gap);
   checkLogicalID( dbcl, null, null, {_id:1}, 1, skipNum - 1, expectIDs);
   println("--check last record logical ID success!");
   
   expectCount = skipNum;
   checkCount( dbcl, null, expectCount);
   println("--check count success!");
   
   //正向pop单个块
   var skipNum = insertNum - 2;
   var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
   pop( dbcl, logicalID[0], 1 );
   
   var expectIDs = [];
   expectIDs.push(recordLength * ( skipNum + 1) + gap);
   checkLogicalID( dbcl, null, null, {_id:1}, 1, null, expectIDs);
   println("--check last record logical ID success!");
   
   expectCount = expectCount - skipNum - 1;
   checkCount( dbcl, null, expectCount);
   println("--check count success!");
   
   //正向pop多个块
   var skipNum = (insertNum - 1) * 3 - 1;
   var logicalID = getLogicalID(dbcl, null, null, {_id:1}, 1, skipNum);
   pop( dbcl, logicalID[0], 1 );
   
   var expectIDs = [];
   expectIDs.push((skipNum + insertNum) * recordLength + 4 * gap);
   checkLogicalID( dbcl, null, null, {_id:1}, 1, null, expectIDs);
   println("--check last record logical ID success!");
   
   expectCount = expectCount - skipNum - 1;
   checkCount( dbcl, null, expectCount);
   println("--check count success!");
   
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();