/******************************************************************************
*@Description : 1.SEQUOIADBMAINSTREAM-274:
*               range-cl is queried error when use operator OR and after split
*
*@Modify list :
*               2014-07-17 pusheng Ding  Init
******************************************************************************/
CHANGEDPREFIX_IDX = CHANGEDPREFIX+"idx" ;

function main()
{
   //get ReplicaGroups
   try{
      var grouplist = Array();
      var cur = db.listReplicaGroups();
      while(cur.next()){
         if(cur.current().toObj()['GroupID'] >= DATA_GROUP_ID_BEGIN ){
            grouplist.push(cur.current().toObj()['GroupName']);
         }
      }
      var groups_num = grouplist.length;
      if(groups_num == 1)
      {
         println("only one ReplicaGroup:" + grouplist + " Skip the testcase");
         return;
      }
   }catch(e)
   {
      println("get ReplicaGroups info fail! rc="+e);
      throw e;
   }
   println("ReplicaGroups: " + grouplist);

   //create CS
   try{
      var varCS = commCreateCS( db, COMMCSNAME, true, "create CS" );
   }catch(e)
   {
      println("can't create CS:" + COMMCSNAME + " rc="+e);
      throw e;
   }
   println("createCS " + COMMCSNAME + " finished");

   //create rangeCL
   try{
      var varCL = varCS.createCL(COMMCLNAME,{ShardingKey:{b:1},ShardingType:'range',ReplSize:0});
      var sn1 = db.snapshot(8,{Name:COMMCSNAME+"."+COMMCLNAME});
      var sourceGroup = sn1.current().toObj()['CataInfo'][0]['GroupName'];
   }catch(e)
   {
      println("can't create range-CL:" + COMMCLNAME + " rc="+e);
      throw e;
   }
   println("createCL " + COMMCLNAME + " at ReplicaGroup:" + sourceGroup + " finished!");

   //insert data
   try{
         var docs = [];
         for(var i=0;i<10000;i++)
         {
            docs.push({a:i-10000,b:i,c:"abcdefghijkl"+i});
         }
         varCL.insert(docs);
   }
   catch(e)
   {
      println("insert-data fail! rc="+e);
   }
   println("insert-data succ!");

   //create index
   try{
      varCL.createIndex(CHANGEDPREFIX_IDX,{a:1},false,false);
   }catch(e)
   {
      println("cat't create index " + CHANGEDPREFIX_IDX + " rc=" + e);
      throw e;
   }
   println("create index finished!");

   //split
   try{
      if(groups_num>1){
         var tarGroupIndex=-1;
         var stepId = 3000;
         var partId = 3;
         var lowId = 0;
         var highId = 0;
         for(var i=0;i<partId;i++){
            tarGroupIndex++;
            if(tarGroupIndex == groups_num)
               tarGroupIndex=0;
            if(grouplist[tarGroupIndex]==sourceGroup)
            {
               i--;
               continue;
            }
            lowId = i*stepId;
            highId = (i+1)*stepId;
            varCL.splitAsync(sourceGroup, grouplist[tarGroupIndex],{b:lowId},{b:highId});
            println(COMMCLNAME+" split from "+sourceGroup+" to "+ grouplist[tarGroupIndex]+" {b:"+lowId+"} {b:"+highId+"}");
         }
         println("split success!");
      }
      else{
         println("can't split to groups!groupsNum is "+groups_num);
      }
   }catch(e)
   {
      println("split fail! rc="+e);
      throw e;
   }

   //select * from ... where a<=-9000 or a>=-1000 or b=2000
   try{
      db.setSessionAttr({PreferedInstance:"M"});
      var sel = varCL.find({$or:[{a:{$lte:-9000}},{a:{$gte:-1000}}, {b:2000}]}).sort({a:1});
      var size=0;
      var flag=true;
      var expected = 2002;
      while(sel.next())
     {
      size++;
      if(size>expected)
      {
         flag = false;
         throw 1;
      }
      var ret = sel.current();
      if(!(ret.toObj()['a']<=-9000 || ret.toObj()['a']>=-1000 || ret.toObj()['b']==2000))
      {
         flag = false;
         throw 2;
      }
     }
     sel.close();
     if(flag && size!=expected)
     {
      flag = false;
      throw 1;
     }
   }catch(e){
      if(e!=1 && e!=2)
      {
         println("select data fail! rc="+e);
         throw e;
      }
      else if(e==1)
      {
         println("return rows not expected! expected:"+expected+" return:"+ size +(size>expected?" or more":""));
         throw e;
      }
      else if(e==2)
      {
         println("return incorrect record! " + ret);
         throw e;
      }
   }
}

try
{
   if( true == commIsStandalone( db ) )
   {
      println( "run mode is standalone" );
   }
   else
   {
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                  "drop CL in the beginning" );
      main();
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                  "drop CL in the end" );
      db.close();
   }
}
catch( e )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop CL in the end, wrong" );
   db.close();
   throw e ;
}
