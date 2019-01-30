/******************************************************************************
@Description :    seqDB-17736:    increment为负值，插入值是序列的currentValue 
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
    
   var clName = COMMCLNAME + "_17736";
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
   
   //coordB指定自增字段值为本coord缓存的nextValue插入记录
   var insertR1 = {a : 1001, id : -1001};
   coordBcl.insert(insertR1);
   expRecs.push(insertR1);
   
   var clID = getCLID(COMMCSNAME, clName);
   var sequenceName = "SYS_" + clID + "_id_SEQ";
   var cursor = db.snapshot(SDB_SNAP_SEQUENCES, { Name : sequenceName });
   if( cursor.current().toObj().CurrentValue !== -2002)
   {
      throw "failed!";
   }
   
   for(var i = 0; i < coordNodes.length; i++)
   {
      var coord = new Sdb(coordNodes[i]);
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      for(var j = 0; j < 110; j++)
      {
         cl.insert({a : j});
         if(i == 1)
         {
            if(j < 100)
            {
               expRecs.push({a : j, id : -1002 + j*increment});  
            }else
            {
               expRecs.push({a : j, id : -1102 + (j-100)*increment + i*acquireSize*increment});  
            }
         }else
         {
            if(j < 99)
            {   
               expRecs.push({a : j, id : -2 + j*increment + i*acquireSize*increment});
            }else
            {
               expRecs.push({a : j, id : -1102 + (j-99)*increment + i*acquireSize*increment});
            }
         }
      }
   }
   
   var rc = coordBcl.find();
   checkRec(rc, expRecs);
   
   commDropCL( db, COMMCSNAME, clName );
}
main();