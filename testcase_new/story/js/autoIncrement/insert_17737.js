/******************************************************************************
@Description :    seqDB-17737:     increment为负值，插入值是序列的cachedValue 
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
    
   var clName = COMMCLNAME + "_17737";
   var increment = -1;
   var acquireSize = 100;
   var startValue = 350;
   var maxValue = 500;
   
   var coordB = new Sdb(coordNodes[1]);
   commDropCL( coordB, COMMCSNAME, clName );
   
   var coordBcl = commCreateCLByOption( coordB, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, 
                                        AcquireSize : acquireSize, StartValue : startValue, MaxValue : maxValue } } );
   commCreateIndex( coordBcl, "a", {id : 1}, true )
   
   var expRecs = [];
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : i});
      expRecs.push({a : i, id : 350 - i*acquireSize});
   }

   //coordB指定自增字段值为序列的cachedValue插入记录
   var insertR1 = {a : 300, id : 300};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1); 

   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 80; j++)
      {
         cl.insert({a : j});
         if(i == 0 && j >= 49)
         {
            expRecs.push({a : j, id : 50 - (j-49)}); 
         }else
         {
            expRecs.push({a : j, id : 349 - j - i*acquireSize}); 
         }
      }
   }
   
   var rc = coordBcl.find();
   checkRec(rc, expRecs);
   
   commDropCL( db, COMMCSNAME, clName );
}
main();