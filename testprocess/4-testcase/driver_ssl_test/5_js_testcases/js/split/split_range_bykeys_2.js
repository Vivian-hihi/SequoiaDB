/******************************************************************************
@Description : 1. split range-collection by keys after insert data
@Modify list :
               2014-07-04  pusheng Ding  Init
               2014-07-08  xiaojun Hu    Changed
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;

function main()
{
   try{
      var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
   }catch(e)
   {
      println("can't connect to db");
      throw e;
   }

   try{
      db.dropCS( CSPREFIX_CS );
   }catch( e ){}

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
      var varCS = db.createCS(CSPREFIX_CS);
   }catch(e)
   {
      println("can't create CS:" + CSPREFIX_CS + " rc="+e);
      throw e;
   }
   println("createCS " + CSPREFIX_CS + " finished");

   //create CL
   try{
      var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{a:1,b:1},ShardingType:"range",ReplSize:3,Compressed:true});
      var sn1 = db.snapshot(8,{Name:CSPREFIX_CS+"."+CSPREFIX_CL});
      var sourceGroup = sn1.current().toObj()['CataInfo'][0]['GroupName'];
   }catch(e)
   {
      println("can't create CL:" + CSPREFIX_CL + " rc="+e);
      throw e;
   }	
   println("createCL " + CSPREFIX_CL + " at ReplicaGroup:" + sourceGroup + " finished");

   //insert data
   try{
      var partitionA = 3;
      var partitionB = 2;
      var stepA = 1;
      var stepB = 2;
      var c = 0;
      for(i=0;i<=partitionA;i++)
      {
         for(j=0;j<=partitionB;j++)
         {
            for(var k=0; k<=stepB;k++)
            {
               varCL.insert({a:i*stepA,b:j*stepB+k,c:c++});
            }
         }
      }
   }catch(e)
   {
      println("insert-data to " + CSPREFIX_CL + " fail! rc="+e);
   }
   println("insert-data " + CSPREFIX_CL + " finish!");

   //split ({a:1,b:1000} {a:1,b:2000} {a:2,b:1000} {a:2,b:2000} {a:3,b:1000} {a:3,b:2000})
   try{
      var tarGroupIndex=-1;
      var lowA = 0;
      var lowB = 0;
      var highA = 0;
      var highB = 0;
      for(var i=0; i<partitionA; i++)
      {
         for(var j=0; j<partitionB; j++)
         {
            tarGroupIndex++;
            if(tarGroupIndex == group_num)
               tarGroupIndex=0;
            if(grouplist[tarGroupIndex]==sourceGroup)
            {
               j--;
               continue;
            }
            lowA = (i+1)*stepA;
            lowB = (j+1)*stepB;
            highA = (i+1)*stepA;
            highB = (j+2)*stepB;
            varCL.split(sourceGroup, grouplist[tarGroupIndex],{a:lowA, b:lowB},{a:highA, b:highB});
            println(CSPREFIX_CL+" split from "+sourceGroup+" to "+ grouplist[tarGroupIndex]+" {a:"+lowA+",b:"+lowB+"} {a:"+highA+",b:"+highB+"}");
         }
      }
   }catch(e)
   {
      println("split " + CSPREFIX_CL + " fail! rc="+e);
      throw e;
   }
   println("split " + CSPREFIX_CL + " finish!");

   //select * from bar where a=1 and b=5
   //expect one record
   try{
      var sel = varCL.find({a:{$et:1},b:{$et:5}});
      var size=0;
      var flag=false;
      while(sel.next())
      {
         size++;
         if(size>100)
            break;
         var ret = sel.current();
         if(ret.toObj()['a']==1 && ret.toObj()['b']==5 && ret.toObj()['c']==16)
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
         println("expected:{a:1,b:5,c:16}");
         println("returned:"+ret);
      }
      else
         println("select " + CSPREFIX_CL + " fail! rc="+e);
      throw e;
   }
   println("data-verify succ!");

   //clean test-env
   try{
      varCS.dropCL(CSPREFIX_CL);
      db.dropCS(CSPREFIX_CS);
   }catch(e)
   {
      println("clean test-env fail!");
      throw e;
   }
   println("clean test-env succ!");
}

// Add inspect standalone run mode
try
{
   var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   main();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
