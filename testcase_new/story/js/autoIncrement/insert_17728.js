/******************************************************************************
@Description :    seqDB-17728:increment为负值，插入值小于当前coord缓存范围，但在其它coord缓存范围内 
@Modify list :   2018-1-25    Zhao Xiaoni  Init
******************************************************************************/
function main()
{
    var coordNodes = getCoordNodeNames();
    if(coordNodes.length < 3 || commIsStandalone( db ))
    {
       println("Deploy is standalone or coord nodes is less than 3!");
       return;
    }
    
   var clName = COMMCLNAME + "_17728";
   var increment = -1;
   var acquireSize = 100;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, AcquireSize : acquireSize } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true )
   
   var expRecs = [];
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : i});
      expRecs.push({a : i, id : -1 - i*acquireSize});
   }
   
   //coordB指定自增字段值小于当前coord缓存范围，但在其它coord缓存范围内插入记录
   var insertR1 = {a : 5, id : -5};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   var coordA = new Sdb(coordNodes[0]);
   for(var i = 0; i < 4; i++)
   {
      coordAcl = coordA.getCS(COMMCSNAME).getCL(clName);
      coordAcl.insert({a : i});
      if(i < 3)
      {
          expRecs.push({a : i, id : -2 - i});
      }else
      {
          expRecs.push({a : 3, id : -301});
      }
   }
   
   //coordB、coordC不指定自增字段插入
   for(var i = 1; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : i});
      expRecs.push({a : i, id : - 2 - i*acquireSize});
   }
   
   var rc = coordAcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();
