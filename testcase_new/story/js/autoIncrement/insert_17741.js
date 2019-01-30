/******************************************************************************
@Description :    seqDB-17741:increment为负值，插入值比序列的MinValue稍大，再次插入自增字段超过MinValue  
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
    
   var clName = COMMCLNAME + "_17741";
   var increment = -2;
   var cacheSize = 500;
   var acquireSize = 100;
   var minValue = -1000;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, 
                                        CacheSize : 500, AcquireSize : acquireSize, MinValue : minValue, Cycled : true } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true );
   
   var expRecs = [];
   coordBcl.insert({a : 1});
   expRecs.push({a : 1, id : -1});
   
   //coordB指定自增字段值比MinValue稍大插入记录,此时coord上的所有缓存被丢弃
   var insertR1 = {a : 999, id : -999};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   //coordB不指定自增字段再次插入记录，翻转获取缓存[-200,-1],唯一索引冲突，再次取缓存范围[-400,-201],自增字段生成值由201开始
   for(var j = 0; j < 20; j++)
   {
      coordBcl.insert({a : "coordA"+j});
      expRecs.push({a : "coordA"+j, id : -201 + j*increment});
   } 
   
   //coordA不指定自增字段插入
   var coordA = new Sdb(coordNodes[0]);
   var coordAcl = coordA.getCS(COMMCSNAME).getCL(clName);
   for(var j = 0; j < 50; j++)
   {
      coordAcl.insert({a : "coordA"+j});
      expRecs.push({a : "coordA"+j, id : -401 + j*increment});
   } 
   
   //coordC不指定自增字段插入
   var coordC = new Sdb(coordNodes[2]);
   var coordCcl = coordC.getCS(COMMCSNAME).getCL(clName);
   for(var j = 0; j < 50; j++)
   {
      coordCcl.insert({a : "coordC"+j});
      expRecs.push({a : "coordC"+j, id : -601 + j*increment});
   } 
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();