/******************************************************************************
@Description :    seqDB-17739:increment为负值，不允许翻转，插入值是序列的MinValue 
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
    
   var clName = COMMCLNAME + "_17739";
   var increment = -1;
   var cacheSize = 500;
   var acquireSize = 100;
   var minValue = -500;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, 
                                        CacheSize : 500, AcquireSize : acquireSize, MinValue : minValue } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true );
   
   var expRecs = [];
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : i});
      expRecs.push({a : i, id : -1 - i*acquireSize});
   }
   
   //coordB指定自增字段值为序列的Minvalue插入记录,此时coord上的所有缓存被丢弃
   var insertR1 = {a : 500, id : minValue};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   //coordA不指定自增字段插入，待coordA缓存耗尽后，超范围插入失败
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
      if(e != -325)
      {
         throw e;
      }
   }

   //coordB超范围插入失败
   try
   {
      coordBcl.insert({a : "coordB"+j});
   }catch(e)
   {
      if(e != -325)
      {
         throw e;
      }
   }
   
   //coordC不指定自增字段插入，待coordC缓存耗尽后，超范围插入失败
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
      if(e != -325)
      {
         throw e;
      }
   }
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();