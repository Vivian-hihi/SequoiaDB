/******************************************************************************
@Description :    seqDB-17740: increment为负值，插入值比序列的MinValue稍大，再次插入自增字段未超MinValue  
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
    
   var clName = COMMCLNAME + "_17740";
   var increment = -1;
   var cacheSize = 500;
   var acquireSize = 100;
   var minValue = -500;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, 
                                        CacheSize : 500, AcquireSize : acquireSize, MinValue : minValue, Cycled : true } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true );
   
   var expRecs = [];
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : i});
      expRecs.push({a : i, id : -1 - i*acquireSize});
   }
   
   //coordB指定自增字段值比MinValue稍大插入记录,此时coord上的所有缓存被丢弃
   var insertR1 = {a : 490, id : -490};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   //coordA不指定自增字段插入
   var coordA = new Sdb(coordNodes[0]);
   var coordAcl = coordA.getCS(COMMCSNAME).getCL(clName);
   for(var j = 0; j < 50; j++)
   {
      coordAcl.insert({a : "coordA"+j});
      expRecs.push({a : "coordA"+j, id : -2 + j*increment});
   } 

   var coordB = new Sdb(coordNodes[1]);
   var coordBcl = coordB.getCS(COMMCSNAME).getCL(clName);
   for(var j = 0; j < 10; j++)
   {
      coordBcl.insert({a : "coordB"+j});
      expRecs.push({a : "coordB"+j, id : -491 + j*increment});
   }
   
   //coordB生成自增字段值达到minValue后翻转，重新取缓存范围[-200,-1],此时需要生成id值为-1,唯一索引冲突，再次取缓存范围[-400,-201],
   //此时生成id值为-101，再次唯一索引冲突后报错-38
   try
   {
      coordBcl.insert({a : "coordB"+j});   
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
      coordCcl.insert({a : "coordC"+j});
      expRecs.push({a : "coordC"+j, id : -202 + j*increment});
   }  
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();