/***************************************************************************
@Description :seqDB-16023 :同时修改多个自增字段的属性值 
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
   
   var clName = COMMCLNAME + "_16023"; 
   var fieldName1 = "id1";  
   var fieldName2 = "id2"; 
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var coordNodes = getCoordNodeNames();
   var coord1 = new Sdb(coordNodes[0]);
   var dbcl = commCreateCLByOption(coord1, COMMCSNAME, clName, {AutoIncrement:[{Field:fieldName1},{Field:fieldName2}]});
   
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
         expR.push({a:i, id1:1 + k*1000 + i, id2:1 + k*1000 + i});
      }
      cl.insert(doc);
      coord.close();
   }
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert success");
   
   var increment1 = 10;
   var cacheSize1 = 32;
   var acquireSize1 = 10;
   var generated1 = "strict";
   var increment2 = 11;
   var cacheSize2 = 33;
   var acquireSize2 = 11;
   dbcl.setAttributes({AutoIncrement:[{Field:fieldName1, Increment:increment1, CacheSize:cacheSize1, AcquireSize:acquireSize1, Generated:generated1},
                                    {Field:fieldName2, Increment:increment2, CacheSize:cacheSize2, AcquireSize:acquireSize2}]});
   var clID = getCLID(COMMCSNAME, clName);
   var clSequenceName1 = "SYS_" + clID + "_" + fieldName1 + "_SEQ";
   var clSequenceName2 = "SYS_" + clID + "_" + fieldName2 + "_SEQ";
   var expIncrementArr = [{Field:fieldName1, SequenceName:clSequenceName1, Generated:generated1},
                          {Field:fieldName2, SequenceName:clSequenceName2}];
   checkAutoIncrementonCL(COMMCSNAME, clName, expIncrementArr);
   println("---check cl autoIncrement success");
   
   var currentValue1 = coordNum * 1000 + 1;
   var clExpSequenceObj = {Increment:increment1, CacheSize:cacheSize1, AcquireSize:acquireSize1, CurrentValue:currentValue1};
   checkSequence(clSequenceName1, clExpSequenceObj);
   println("---check cl sequence1 success");
   var currentValue2 = coordNum * 1000 + 1;
   var clExpSequenceObj = {Increment:increment2, CacheSize:cacheSize2, AcquireSize:acquireSize2, CurrentValue:currentValue2};
   checkSequence(clSequenceName2, clExpSequenceObj);
   println("---check cl sequence2 success");
   for(var k=0; k<coordNum; k++ )
   {
      var coord = new Sdb(coordNodes[k]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      var doc = [];
      for(var i=0;i<100;i++)
      {
         if(k==0)
         {
            var expId1 = currentValue1 + increment1 -1 + Math.ceil(100/acquireSize1)*acquireSize1*increment1*k + increment1*i;
			   var expId2 = currentValue2 + increment2 -1 + Math.ceil(100/acquireSize2)*acquireSize2*increment2*k + increment2*i;
         }else
         {
            //alter操作修改Generated属性时，本coord插入不会更新版本号，其他coord插入时会更新版本号(跟开发确认无可避免)，自增字段产生空洞(空洞记录数个值)SEQUOIADBMAINSTREAM-3895
            var expId1 = currentValue1 + increment1 -1 + Math.ceil(100 *2/acquireSize1)*acquireSize1*increment1*k + increment1*i;
			   var expId2 = currentValue2 + increment2 -1 + Math.ceil(100 *2/acquireSize2)*acquireSize2*increment2*k + increment2*i;
			   //expId1 += increment1;
			   //???
            expId2 += increment2;
         }
         
         doc.push({a:i});
         expR.push({a:i, id1:expId1, id2:expId2});
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