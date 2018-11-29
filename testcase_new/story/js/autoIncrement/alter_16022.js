/***************************************************************************
@Description :seqDB-16022 :修改单个自增字段的所有属性 
@Modify list :
              2018-10-25  zhaoyu  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_16022"; 
   var fieldName = "id";  
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var coordNodes = getCoordNodeNames();
   var coord1 = new Sdb(coordNodes[0]);
   var dbcl = commCreateCLByOption(coord1, COMMCSNAME, clName, {AutoIncrement:{Field:fieldName}});
   
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
         expR.push({a:i, id:1 + k*1000 + i});
      }
      cl.insert(doc);
      coord.close();
   }
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert success");
   
   var increment = 10;
   var startValue = 100;
   var minValue = -100;
   var currentValue = -10
   var maxValue = 1000000;
   var cacheSize = 32;
   var acquireSize = 10;
   var cycled = true;
   var generated = "strict";
   dbcl.setAttributes({AutoIncrement:{Field:fieldName, Increment:increment, StartValue:startValue, MinValue:minValue, MaxValue:maxValue,
                       CacheSize:cacheSize, AcquireSize:acquireSize, Cycled:cycled, Generated:generated, CurrentValue:currentValue}});
   var clID = getCLID(COMMCSNAME, clName);
   var clSequenceName = "SYS_" + clID + "_" + fieldName + "_SEQ";
   var expIncrementArr = [{Field:fieldName, SequenceName:clSequenceName, Generated:generated}];
   checkAutoIncrementonCL(COMMCSNAME, clName, expIncrementArr);
   println("---check cl autoIncrement success");
   
   var clExpSequenceObj = {Increment:increment,StartValue:startValue, MinValue:minValue, MaxValue:maxValue, CacheSize:cacheSize,
                           AcquireSize:acquireSize, Cycled:cycled, CurrentValue:currentValue};
   checkSequence(clSequenceName, clExpSequenceObj);
   println("---check cl sequence success");
   
   for(var k=0; k<coordNum; k++ )
   {
      var coord = new Sdb(coordNodes[k]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      var doc = [];
      for(var i=0;i<100;i++)
      {
         doc.push({a:i});
         if(k==0)
         {
            expR.push({a:i, id: currentValue + increment + Math.ceil(100/acquireSize)*acquireSize*increment*k + increment*i});
         }else
         {
            //alter操作修改Generated属性时，本coord插入不会更新版本号，其他coord插入时会更新版本号(跟开发确认无可避免)，自增字段产生空洞(空洞记录数个值)SEQUOIADBMAINSTREAM-3895
            expR.push({a:i, id: currentValue + increment + Math.ceil(100*2/acquireSize)*acquireSize*increment*k + increment*i});
         }
         
      }
      cl.insert(doc);
      coord.close();
   }
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert after alter autoIncrement success");
   commDropCL(coord1, COMMCSNAME, clName, true, true);
   coord1.close();
}
main()