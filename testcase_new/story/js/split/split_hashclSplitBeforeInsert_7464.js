/******************************************************************************
@Description : 1. split hash-collection by keys after insert data
@Modify list :
               2014-07-04  pusheng Ding  Init
               2014-07-08  xiaojun Hu    Changed
******************************************************************************/
function main()
{
   //@ clean before
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop CL in the beginning" );
   //get ReplicaGroups
   try{
      var grouplist = Array();
      var cur = db.listReplicaGroups();
      while(cur.next()){
         if(cur.current().toObj()['GroupID'] >= DATA_GROUP_ID_BEGIN ){
            grouplist.push(cur.current().toObj()['GroupName']);
         }
      }
      var group_num = grouplist.length;
      if(group_num == 1)
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

   //create CL
   try{
      var varCL = varCS.createCL(COMMCLNAME,{ShardingKey:{a:1,b:-1},ShardingType:"hash",ReplSize:3,Partition:4096});
      var sn1 = db.snapshot(8,{Name:COMMCSNAME+"."+COMMCLNAME});
      var sourceGroup = sn1.current().toObj()['CataInfo'][0]['GroupName'];
   }catch(e)
   {
      println("can't create CL:" + COMMCLNAME + " rc="+e);
      throw e;
   }
   println("createCL " + COMMCLNAME + " at ReplicaGroup:" + sourceGroup + " finished");

   //insert data
   try{
      var c = 0;
      var docs = [];
      for(i=1;i<=100;i++)
      {
         for(j=1;j<=100;j++)
         {
            docs.push({a:i,b:j,c:c++});
         }
      }
      varCL.insert(docs);
   }catch(e)
   {
      println("insert-data to " + COMMCLNAME + " fail! rc="+e);
   }
   println("insert-data " + COMMCLNAME + " finish!");

   //split 
   try{
      var tarGroupIndex = -1;
      for(var i=1; i<4; i++)
      {
         tarGroupIndex++;
         if(tarGroupIndex == group_num)
            tarGroupIndex=0;
         if(sourceGroup == grouplist[tarGroupIndex])
         {
            i--;
            continue;
         }
         lowP = i*1024;
         highP = (i+1)*1024;
         varCL.split(sourceGroup,grouplist[tarGroupIndex],{Partition:lowP},{Partition:highP});
         println(COMMCLNAME+" split from "+sourceGroup+" to "+ grouplist[tarGroupIndex]+" {Partition:"+lowP+ "} {Partition:"+highP+"}");
      }
   }catch(e)
   {
      println("split " + COMMCLNAME + " fail! rc="+e);
      throw e;
   }
   println("split " + COMMCLNAME + " finish!");

   //select * from bar where a=1 and b=5
   //expect one record
   try{
      db.setSessionAttr({PreferedInstance:"M"});
      var sel = varCL.find({a:{$et:1},b:{$et:5}});
      var size=0;
      var flag=false;
      while(sel.next())
      {
         size++;
         if(size>100)
            break;
         var ret = sel.current();
         if(ret.toObj()['a']==1 && ret.toObj()['b']==5 && ret.toObj()['c']==4)
            flag = true;
      }
      if(size!=1)
      {
         throw -1;
      }
      if(!flag)
      {
         throw -2;
      }
   }catch(e)
   {
      if(e==-1)
         println("result-records count not expected. expect:1 return:"+size);
      else if(e==-2)
      {
         println("record not expected!");
         println("expected:{a:1,b:5,c:4}");
         println("returned:"+ret);
      }
      else
         println("select " + COMMCLNAME + " fail! rc="+e);
      throw e;
   }
   println("data-verify succ!");
   //@ clean after
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop CL in the beginning" );
}

try
{
   if( true == commIsStandalone( db ) )
   {
      println( "run mode is standalone" );
   }
   else
   {
      main();
      db.close();
   }
}
catch( e )
{
   throw e ;
}
