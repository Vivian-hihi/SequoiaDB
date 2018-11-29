/***************************************************************************
@Description :seqDB-16024 :同时修改自增字段及其他属性 
@Modify list :
              2018-10-26  zhaoyu  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_16024"; 
   var fieldName = "id";  
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var increment = 12;
   var acquireSize = 11;
   var coordNodes = getCoordNodeNames();
   var coord1 = new Sdb(coordNodes[0]);
   var dbcl = commCreateCLByOption(coord1, COMMCSNAME, clName, {AutoIncrement:{Field:fieldName, Increment:increment, AcquireSize:acquireSize}});
   
   var coordNum = coordNodes.length;
   var expR = [];
   for(var k=0; k<coordNum; k++ )
   {
      var coord = new Sdb(coordNodes[k]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      var doc = [];
      for(var i=0;i<100;i++)
      {
         doc.push({a:i});
         expR.push({a:i, id:1 + Math.ceil(100/acquireSize)*acquireSize*increment*k + increment*i});
      }
      cl.insert(doc);
      coord.close();
   }
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert success");
   
   var cacheSize = 32;
   var acquireSize = 11;
   var generated = "strict";
   var currentValue = 1000*increment + 1;
   dbcl.setAttributes({AutoIncrement:{Field:fieldName, CacheSize:cacheSize, AcquireSize:acquireSize, Generated:generated}, ShardingKey: { a: 1 }, CompressionType:'lzw'});
   var clID = getCLID(COMMCSNAME, clName);
   var clSequenceName = "SYS_" + clID + "_" + fieldName + "_SEQ";
   var expIncrementArr = [{Field:fieldName, SequenceName:clSequenceName, Generated:generated}];
   checkAutoIncrementonCL(COMMCSNAME, clName, expIncrementArr);
   println("---check cl autoIncrement success");
   
   var clExpSequenceObj = {Increment:increment, CacheSize:cacheSize, AcquireSize:acquireSize, CurrentValue:currentValue};
   checkSequence(clSequenceName, clExpSequenceObj);
   println("---check cl sequence success");
   
   checkSnapshot8onCL(COMMCSNAME, clName);
   println("---check cl shardingType and compressType success");
   
   for(var k=0; k<coordNum; k++ )
   {
      var coord = new Sdb(coordNodes[k]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      var doc = [];
      for(var i=0;i<100;i++)
      {
         doc.push({a:i});
         if(k == 0)
         {
            var expID =  1 + Math.ceil(100/acquireSize)*acquireSize*increment*coordNum + Math.ceil(100/acquireSize)*acquireSize*increment*k + increment*i;
	      }else
         {
            //alter操作修改Generated属性时，本coord插入不会更新版本号，其他coord插入时会更新版本号(跟开发确认无可避免)，自增字段产生空洞(空洞记录数个值)SEQUOIADBMAINSTREAM-3895
            var expID = 1 + Math.ceil(100/acquireSize)*acquireSize*increment*coordNum + Math.ceil(100*2/acquireSize)*acquireSize*increment*k + increment*i;
            //???
            expID += increment;
         }
         expR.push({a:i, id: expID});
         
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
   commDropCL(coord1, COMMCSNAME, clName, true, true); 
   coord1.close();
}
main()

function checkSnapshot8onCL(csName, clName)
{
   var obj = db.snapshot(8,{Name:csName + "." + clName}).next().toObj();
   var shardingType = obj.ShardingType;
   var compressionType = obj.CompressionTypeDesc;
   if(shardingType !== "hash" || compressionType !== "lzw")
   {
      println("shardingType:" + shardingType + ",compressionType:" + compressionType + "\n");
      throw "ALTER_CL_ERR";
   }  
}
