/******************************************************************************
@Description :    seqDB-17744: increment为负值，超大范围调整 
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
    
   var clName = COMMCLNAME + "_17744";
   var increment = -1;
   var acquireSize = 100;
   var maxValue = { "$numberLong" : "9223372036854775807" } 
   
   commDropCL( db, COMMCSNAME, clName );
   
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, { AutoIncrement : { Field : "id", Increment : increment, 
                                   AcquireSize : acquireSize, MaxValue : maxValue }});
   commCreateIndex( dbcl, "a", {id : 1}, true );
   
   var expRecs = [];
   var cl = new Array();
   var coord  = new Array();
   for(var i = 0; i < coordNodes.length; i++)
   {
      coord[i] = new Sdb(coordNodes[i]);
      cl[i] = coord[i].getCS(COMMCSNAME).getCL(clName);
      cl[i].insert({a : i});
      expRecs.push({a : i, id : -1 - i*acquireSize});
   }
   
   dbcl.setAttributes({AutoIncrement : {Field : "id", CurrentValue : 2147483647}});
   
   var insertR1 = {a : 3, id : -2147483648 }
   cl[0].insert(insertR1);
   expRecs.push(insertR1);
   
   for(var i = 0; i < coordNodes.length; i++)
   {
      cl[i].insert({a : i});
      expRecs.push({a : i, id : -2147483649 - i*acquireSize});
   }
   
   var rc = dbcl.find().sort({id:1});
   checkRec(rc, expRecs.sort(compare("id")));
   
   commDropCL( db, COMMCSNAME, clName );
}
main();