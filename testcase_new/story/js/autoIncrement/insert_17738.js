/******************************************************************************
@Description :    seqDB-17738:      increment为负值，允许翻转，插入值是序列的MinValue 
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
    
   var clName = COMMCLNAME + "_17738";
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
   
   //coordB指定自增字段值为序列的Minvalue插入记录,此时翻转，coordB缓存为[-100,-1]
   var insertR1 = {a : 500, id : minValue};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   //coordA不指定自增字段插入记录，当插入第99条记录时，coordA缓存耗尽，获取新的缓存[-200,-101]，此时生成id值为-101，唯一索引首次
   //冲突不报错，再次重新获取新的缓存[-201,-300],此时生成id值为-201再次唯一索引冲突才会报错-38;
   try
   {
      var coordA = new Sdb(coordNodes[0]);
      var coordAcl = coordA.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 100; j++)
      {
         coordAcl.insert({a : "coordA"+j});
         expRecs.push({a : "coordA"+j, id : -2 + j*increment});
      }  
   }catch(e)
   {
      if(e != -38)
      {
         throw e;
      }
   }

   //coordB原本缓存的范围为[-101,-1],此时生成id值为-1，唯一索引冲突首次不报错，再次获取新的缓存[-301,-400],因此id的生成值从-301开始
   for(var j = 0; j < 120; j++)
   {
      coordBcl.insert({a : "coordB"+j});
      if(j < 100)
      {
         expRecs.push({a : "coordB"+j, id : -301 + j*increment});   
      }else
      {
         expRecs.push({a : "coordB"+j, id : -401 + (j-100)*increment}); 
      }
      
   }
   
   //coordC不指定自增字段插入记录，id的生成未做调整
   try
   {
      var coordC = new Sdb(coordNodes[2]);
      var coordCcl = coordC.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 120; j++)
      {
         coordCcl.insert({a : "coordC"+j});
         expRecs.push({a : "coordC"+j, id : -202 + j*increment});
      }  
   }catch(e)
   {
      if(e != -38)
      {
         throw e;
      }
   }
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();