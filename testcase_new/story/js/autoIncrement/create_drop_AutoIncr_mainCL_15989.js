/***************************************************************************
@Description : seqDB-15989:主子表上创建/删除自增字段，删除主表自增字段
@Modify list :
              2018-10-18  zhaoyu  Create
****************************************************************************/
function main()
{
   var dataGroupNames = getDataGroupNames();
   println("dataGroupNames:" + dataGroupNames);
   if(commIsStandalone( db ) || dataGroupNames.length <2)
   {
      println("Deploy is standalone or only one group");
	  return;
   }
   
   var maincsName = COMMCSNAME + "_maincs_15989";
   var mainclName = COMMCLNAME + "_maincl_15989";
   
   var subcsName = COMMCSNAME + "_subcs_15989";
   var subclName1 = COMMCLNAME + "_subcl_15989_1";
   var subclName2 = COMMCLNAME + "_subcl_15989_2";
   var subclName3 = COMMCLNAME + "_subcl_15989_3";
   
   var mainclFullName = maincsName + "." + mainclName;
   var subclFullName1 = maincsName + "." + subclName1;
   var subclFullName2 = subcsName + "." + subclName2;
   var subclFullName3 = subcsName + "." + subclName3;
   
   var maincl;
   var subcl1;
   var subcl2;
   var subcl3;
   var fieldName = "id";

   commDropCS( db, subcsName);
   commDropCS( db, maincsName);
   
   var mainclOption = {IsMainCL: true, ShardingKey: {"a": 1}, ShardingType: "range"};
   maincl = commCreateCLByOption( db, maincsName, mainclName, mainclOption);
   
   var subclOption1 = {ShardingKey: {"a0": 1}, ShardingType: "range", Group: dataGroupNames[0]};
   subcl1 = commCreateCLByOption( db, maincsName, subclName1, subclOption1);
   var subclOption2 = {ShardingKey: {"a0": 1}, ShardingType: "hash", Group: dataGroupNames[0]};
   subcl2 = commCreateCLByOption( db, subcsName, subclName2, subclOption2);
   var subclOption3 = {Group:dataGroupNames[0]};
   subcl3 = commCreateCLByOption( db, subcsName, subclName3, subclOption3);
   
   subcl1.split( dataGroupNames[0], dataGroupNames[1], {a0:1000}, {a0:2000} );
   subcl2.split( dataGroupNames[0], dataGroupNames[1], 50 );
   
   maincl.attachCL( subclFullName1, {LowBound: {a:1}, UpBound:{a:2001}} );
   maincl.attachCL( subclFullName2, {LowBound: {a:2001}, UpBound:{a:4001}} );
   maincl.attachCL( subclFullName3, {LowBound: {a:4001}, UpBound:{a:6001}} );
   
   var mainclCacheSize = 10;
   var mainclAcquireSize = 1;
   var subclIncrement = 10;
   maincl.createAutoIncrement({Field:fieldName, CacheSize:mainclCacheSize, AcquireSize:mainclAcquireSize});
   subcl1.createAutoIncrement({Field:fieldName, Increment:subclIncrement});
   
   var mainclID = getCLID(maincsName, mainclName);
   var mainclSequenceName = "SYS_" + mainclID + "_" + fieldName + "_SEQ";
   var expIncrementArr = [{Field:fieldName, SequenceName:mainclSequenceName}];
   checkAutoIncrementonCL(maincsName, mainclName, expIncrementArr);
   println("---check maincl autoIncrement success");
   
   var subclID = getCLID(maincsName, subclName1);
   var subclSequenceName = "SYS_" + subclID + "_" + fieldName + "_SEQ";
   var expIncrementArr = [{Field:fieldName, SequenceName:subclSequenceName}];
   checkAutoIncrementonCL(maincsName, subclName1, expIncrementArr);
   println("---check subcl autoIncrement success");
   
   var mainExpSequenceObj = {CacheSize:mainclCacheSize, AcquireSize:mainclAcquireSize};
   checkSequence(mainclSequenceName, mainExpSequenceObj);
   println("---check maincl sequence success");
   var subExpSequenceObj = {Increment:subclIncrement};
   checkSequence(subclSequenceName, subExpSequenceObj);
   println("---check subcl sequence success");
   
   var doc = [];
   var expR = [];
   for(var i=1; i<6001; i++)
   {
      doc.push({a:i,b:i});
      expR.push({a:i,b:i,id:i});
   }
   maincl.insert(doc);
   var actR = maincl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert into maincl success");
   
   var doc = [];
   for(var i=1; i<6001; i++)
   {
      doc.push({a:i,a0:i});
      expR.push({a:i,a0:i,id:(i-1)*10+1,});
   }
   subcl1.insert(doc);
   var actR = maincl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert into subcl success");
   
   maincl.dropAutoIncrement({Field:fieldName});
   
   var doc = [];
   for(var i=1; i<6001; i++)
   {
      doc.push({a:i,b:i});
      expR.push({a:i,b:i});
   }
   maincl.insert(doc);
   var actR = maincl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert into maincl after drop increment field success");
   
   var doc = [];
   var j=6001;
   for(var i=1; i<6001; i++)
   {
      doc.push({a:i,a0:i});
      expR.push({a:i,a0:i,id:(j-1)*10+1,});
      j++
   }
   subcl1.insert(doc);
   var actR = maincl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert into subcl after drop increment field success");
   
   commDropCS( db, subcsName);
   commDropCS( db, maincsName);
}
main()