/******************************************************************************
@Description :    seqDB-17732:   increment为负值，插入值大于所有缓存值 
@Modify list :   2018-1-28    Zhao Xiaoni  Init
******************************************************************************/
function main()
{
   var coordNodes = getCoordNodeNames();
   if(coordNodes.length < 3 || commIsStandalone( db ))
   {
      println("Deploy is standalone or coord nodes is less than 3!");
      return;
   }
    
   var clName = COMMCLNAME + "_17732";
   var increment = -2;
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
      for(var j = 0; j < acquireSize; j++)
      { 
         cl.insert({a : j});
         expRecs.push({a : j, id : - 1 + j*increment + i*increment*acquireSize});
      }
   }
   
   //coordB指定自增字段值大于所有缓存值
   var insertR1 = {a : 2, id : -2};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 5; j++)
      {
         cl.insert({a : j});
         expRecs.push({a : j, id : - 601 + j*increment + i*acquireSize*increment});
      }
   }
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();