/***************************************************************************
@Description : seqDB-16025:hash分区表修改自增字段属性 
@Modify list :
              2018-10-26  zhaoyu  Create
****************************************************************************/
function main()
{
   var dataGroupNames = getDataGroupNames();
   if(commIsStandalone( db ) || dataGroupNames.length <2)
   {
      println("Deploy is standalone or only one group");
	  return;
   }
   
   var csName = COMMCSNAME + "_16025";
   var clName = COMMCLNAME + "_16025";
   var fieldName = "id";
   var domainName = "domain_16025";
   commDropCS( db, csName);
   try
   {
      db.dropDomain(domainName); 
   }catch(e)
   {
      if(-214 !== e)
      {
         throw e;
      }
   }
   
   db.createDomain(domainName,[dataGroupNames[0], dataGroupNames[1]], {AutoSplit: true});
   commCreateCS( db, csName, null, null, {Domain:domainName} );
   var cacheSize = 20;
   var acquireSize = 11;
   var increment =10;
   var dbcl = commCreateCLByOption(db, csName, clName, {ShardingKey:{id:1}, 
                                                        AutoIncrement:{Field:fieldName, CacheSize:cacheSize, AcquireSize:acquireSize, Increment:increment}});
   
   var clID = getCLID(csName, clName);
   var sequenceName = "SYS_" + clID + "_" + fieldName + "_SEQ";
   var expIncrementArr = [{Field:fieldName, SequenceName:sequenceName}];
   checkAutoIncrementonCL(csName, clName, expIncrementArr);
   println("---check cl autoIncrement success");
   
   var expSequenceObj = {AcquireSize:acquireSize, CacheSize:cacheSize, Increment:increment};
   checkSequence(sequenceName, expSequenceObj);
   println("---check cl sequence success");
   
   var doc = [];
   var expR = [];
   for(var i=0; i<100; i++)
   {
      doc.push({a:i,b:i});
      expR.push({a:i,b:i,id:i*increment +1});
   }
   dbcl.insert(doc);
   
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert success");
   
   //插入100条记录，catalog上缓存序列生成了100/cacheSize次,且序列值已用完
   var currentValue = (Math.ceil(100/cacheSize)+1)*cacheSize*increment + 1;
   
   //插入100条记录，coord获取的序列值未用完，alter后会清空coord上未用完的序列值
   var nextValue = Math.ceil(100/acquireSize)*acquireSize*increment + 1;
   var cacheSize = 32;
   var acquireSize = 12;
   var generated = "strict";
   dbcl.setAttributes({AutoIncrement:{Field:fieldName, CacheSize:cacheSize, AcquireSize:acquireSize, Generated:generated}});
   var clID = getCLID(csName, clName);
   var clSequenceName = "SYS_" + clID + "_" + fieldName + "_SEQ";
   var expIncrementArr = [{Field:fieldName, SequenceName:clSequenceName, Generated:generated}];
   checkAutoIncrementonCL(csName, clName, expIncrementArr);
   println("---check cl autoIncrement after alter success");
   
   var clExpSequenceObj = {Increment:increment, CacheSize:cacheSize, AcquireSize:acquireSize, CurrentValue:currentValue};
   checkSequence(clSequenceName, clExpSequenceObj);
   println("---check cl sequence after alter success");
   
   var coordNodes = getCoordNodeNames();
   var coordNum = coordNodes.length;
   for(var k=0; k<coordNum; k++ )
   {
      var coord = new Sdb(coordNodes[k]);
      var cl = coord.getCS(csName).getCL(clName);
      //alter操作会变更集合版本号，插入时会取2次seqence值，SEQUOIADBMAINSTREAM-3895,通过find操作更新版本号
      var cursor = cl.find();
      while(cursor.next()){}
      var doc = [];
      for(var i=0;i<100;i++)
      {
         doc.push({a:i});
         expR.push({a:i, id: nextValue + Math.ceil(100/acquireSize)*acquireSize*increment*k + increment*i});
      }
      cl.insert(doc);
      coord.close();
   }
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert after alter autoIncrement success");
   
   try
   {
      dbcl.insert({id:"a"});
      throw "NEED_INSERT_ERR";  
   }catch(e)
   {
      if(-6 !== e)
      {
         throw e;
      }
   }
   println("---check insert after alter generated success");
   commDropCS( db, csName);
   db.dropDomain(domainName);
}
main()