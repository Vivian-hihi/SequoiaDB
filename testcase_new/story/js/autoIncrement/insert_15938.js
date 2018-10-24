/***************************************************************************
@Description :seqDB-15938 :不指定自增字段批量插入记录
@Modify list :
              2018-10-16  zhaoyu  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
	  return;
   }
   
   var clName = COMMCLNAME + "_15938";   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var acquireSize = 1;
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, {AutoIncrement:{Field:"id",AcquireSize:acquireSize}});
   commCreateIndex(dbcl, "id", {id:1}, true, true);
   
   var coordNodes = getCoordNodeNames();
   var coordNum = coordNodes.length;
   var expR = [];
   for(var k=0; k<coordNum; k++ )
   {
      var coord = new Sdb(coordNodes[k]);
      //coord.invalidateCache();
      var cl = coord.getCS(COMMCSNAME).getCL(clName);
      for(var j=0; j<2; j++)
      {
         var doc = [];
         for(var i=1; i<2001; i++)
         {
            doc.push({a:i, b:i, c:i + "test"});
            expR.push({a:i, b:i, c:i + "test", id:k*4000 + j*2000 + i});
         }
         cl.insert(doc);
      }
      coord.close();
   }
   
   var actR = dbcl.find().sort({_id:1});
   checkRec(actR, expR);
   println("---check insert success");
   
   commDropCL(db, COMMCSNAME, clName, true, true); 
}
main()