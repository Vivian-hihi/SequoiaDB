/******************************************************************************
@Description :    seqDB-17731:  increment为负值，插入值大于当前coord缓存范围，但在其它coord缓存范围内 
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
    
   var clName = COMMCLNAME + "_17731";
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
      expRecs.push({a : i, id : - 1 - i*acquireSize});
   }
   
   //coordB指定自增字段值大于当前coord缓存范围，但在其它coord缓存范围内
   var insertR1 = {a : 205, id : -205};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 5; j++)
      {
         cl.insert({a : j});
         if(i == 0)
         {
            expRecs.push({a : j, id : - 2 - j});
         }
         if(i == 1)
         {
            expRecs.push({a : j, id : -301 - j});  
         }
         if(i == 2)
         {
            if(j < 3)
            {
               expRecs.push({a : j, id : -202 - j}); 
            }else
            {
               expRecs.push({a : j, id : -401 - (j-3)});
            }
         }
      }
   }
   
   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();