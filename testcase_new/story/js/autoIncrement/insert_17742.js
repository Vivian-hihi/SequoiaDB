/******************************************************************************
@Description :    seqDB-17742: increment为负值，插入值比序列的MinValue稍大，MinValue为负边界值，再次插入自增字段未超MinValue 
@Modify list :   2018-1-29    Zhao Xiaoni  Init
******************************************************************************/
function main()
{
   var coordNodes = getCoordNodeNames();
   if(coordNodes.length < 3 || commIsStandalone( db ))
   {
      println("Deploy is standalone or coord nodes is less than 3!");
      return;
   }
    
   var clName = COMMCLNAME + "_17742";
   var increment = -2;
   var acquireSize = 100;
   var sortField = 0;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, 
                                        AcquireSize : acquireSize, Cycled : true } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true );
   
   var expRecs = [];
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : sortField});
      expRecs.push({a : sortField, id : -1 + i*acquireSize*increment});
      sortField++;
   }
   
   //coordB指定自增字段值比MinValue稍大插入记录,此时coord上的所有缓存被丢弃
   var insertR1 = {a : sortField, id : { "$numberLong" : "-9223372036854775805" }};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   sortField++;
   
   //coordA不指定自增字段插入
   var coordA = new Sdb(coordNodes[0]);
   var coordAcl = coordA.getCS(COMMCSNAME).getCL(clName);
   for(var j = 0; j < 50; j++)
   {
      coordAcl.insert({a : sortField});
      expRecs.push({a : sortField, id : -3 + j*increment});
      sortField++;
   } 

   //coordB插入记录，插入成功，消耗完最后一个自增字段值
   var coordB = new Sdb(coordNodes[1]);
   var coordBcl = coordB.getCS(COMMCSNAME).getCL(clName);
   coordBcl.insert({a : sortField});
   expRecs.push({a : sortField, id : { "$numberLong" : "-9223372036854775807" }});
   sortField++;
   
   //coordB再次插入生成自增字段值大于minValue因此翻转，重新取缓存范围[-200,-1],此时需要生成id值为-1,唯一索引冲突，再次取缓存范围[-400,-201],
   //此时生成id值为-101，再次唯一索引冲突后报错-38
   try
   {
      coordBcl.insert({a : sortField}); 
      throw "coordB insert error!";
   }catch(e)
   {
      if(e != -38)
      {
         throw e;
      }
   }
   
   //coordC不指定自增字段插入
   var coordC = new Sdb(coordNodes[2]);
   var coordCcl = coordC.getCS(COMMCSNAME).getCL(clName);
   for(var j = 0; j < 50; j++)
   {
      coordCcl.insert({a : sortField});
      expRecs.push({a : sortField, id : -403 + j*increment});
      sortField++;
   }  
   
   var rc = coordBcl.find().sort({a:1});
   checkRec(rc, expRecs.sort(compare("a")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();