/******************************************************************************
@Description :    seqDB-17733:    increment为负值，在序列未使用时（无缓存状态）插入值 
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
    
   var clName = COMMCLNAME + "_17733";
   var increment = -1;
   var acquireSize = 100;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, AcquireSize : acquireSize } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true )
   
   var expRecs = [];
   var insertR1 = {a : 20, id : -20};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 50; j++)
      {
         cl.insert({a : j});
         if(i == 0)
         {
            expRecs.push({a : j, id : -1 + j*increment + (i+1)*increment*acquireSize});
         }
         if(i == 1)
         {
            expRecs.push({a : j, id : -21 + j*increment});
         }
         if(i == 2)
         {
            expRecs.push({a : j, id : -1 + j*increment + i*increment*acquireSize});
         }
      }
   }
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();