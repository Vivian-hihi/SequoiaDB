/******************************************************************************
@Description :    seqDB-17727:increment为负值，插入值小于所有缓存值
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
    
   var clName = COMMCLNAME + "_17727";
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
   
   //coordB指定自增字段值小于所有coord缓存的最小值插入记录
   var insertR1 = {a : 500, id : -500};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);     
   
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      cl.insert({a : i});
      if(i == 1)
      {
         expRecs.push({a : i, id : -501});
      }else
      {
         expRecs.push({a : i, id : -2 - i*acquireSize});
      }
   }

   var rc = coordBcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( coordB, COMMCSNAME, clName );
}
main();
