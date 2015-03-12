//check split statement
//normally case
function getSourceGroupName(CS_Name,CL_Name)
{
   try
   {
      var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
      var allCollections = db.snapshot(8).toArray() ;
      var CS_CL = CS_Name+"."+CL_Name;
      var GroupName = [];
      for( var i = 0 ; i<allCollections.length ; i++ )
      {
         //println( eval("("+allCollections[i]+")") ) ;
         var eval_CL = eval("("+allCollections[i]+")") ;
         //println( allCollections[i] ) ;
         if(eval_CL["Name"] == CS_CL)
         {
            //println( eval_CL.Name ) ;
            for(var j = 0;j<eval_CL["CataInfo"].length;j++)
            {
               GroupName.push(eval_CL["CataInfo"][j]["GroupName"]) ;
            }
            return GroupName;
         }
      }
      return GroupName;
   }
   catch( e )
   {
      println( "failed to get source group, rg = " + e ) ;
      throw e ;
   }
}
function getTheSameRoleGroups(Role)
{
   try
   {
      var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME) ;
      var allGroups  = db.listReplicaGroups().toArray() ;
      var RoleGroupNumbers = 0 ;
      var Groups = [];
      for ( var i = 0; i<allGroups.length; i++ )
      {
         var eval_node = eval("("+allGroups[i]+")") ;
         if(eval_node["Role"] == Role)
         {
            var Group = new Array() ;
            Group["GroupName"] = eval_node["GroupName"];
            Group["GroupID"] = eval_node["GroupID"];
            Groups.push(Group) ;
         }
      }
      return Groups;
   }
   catch( e )
   {
      println( "failed to get same role group, rc = " + e ) ;
      throw e;
   }
}
function main()
{
   try
   {
      var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
      db.listReplicaGroups() ;
   }
   catch( e )
   {
      if( e == -159 )
      {
         println( "can't run in standalone" ) ;
         return ;
      }
      println( "fail to check standalone" ) ;
      throw e ;
   }
   CSPREFIX_CS = CSPREFIX+"foo";
   CSPREFIX_CL = CSPREFIX+"bar";
   var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME) ;
   try
   {
      var dataRGs = getTheSameRoleGroups(0) ;
      println("The numbers of DataGroups is "+dataRGs.length) ;
      if(dataRGs.length<2)
      {
         println("Don't have enough DataGroup to test split_testcase") ;
         return -1;
      }
   }
   catch(e)
   {
      println("Can't get the numbers of DataGroup: "+e) ;
      throw e;
   }

   try
   {
      db.dropCS(CSPREFIX_CS) ;
   }
   catch(e)
   {
      println("At first,cann't drop the CS : "+CSPREFIX_CS+" error code is:"+e) ;
   }

   try
   {
      var CS = db.createCS(CSPREFIX_CS) ;
      var CL = CS.createCL(CSPREFIX_CL,{ShardingKey:{id:1},ShardingType:"range",ReplSize:0,Compressed:true}) ;
   }
   catch(e)
   {
      println("Cann't createCS or createCL, the error code is:"+e) ;
      throw e;
   }
   println( "success to create collection" ) ;
   try
   {
      var sourceDataGroupName = getSourceGroupName(CSPREFIX_CS,CSPREFIX_CL) ;
   }
   catch(e)
   {
      println( "failed to get source group name, rc = " + e ) ;
      throw e;
   }
   try
   {
      for( i = 1 ; i <= 500*dataRGs.length ; i++ )
      {
         CL.insert({id:i}) ;
      }
   }
   catch(e)
   {
      println("insert error") ;
      throw e;
   }
   try
   {
      var desDataGroupName = [];//all split target group
      for(var i = 0;i<dataRGs.length;i++)
      {
         var isIn = false;
         for(var j = 0;j<sourceDataGroupName.length;j++)
         {
            if(sourceDataGroupName[j] == dataRGs[i]["GroupName"])
            {
               isIn = true;
               break;
            }
         }
         if(!isIn)
         {
            desDataGroupName.push(dataRGs[i]["GroupName"]) ;
         }
      }
      println("desDataGroupName is "+desDataGroupName) ;

      //split
      var percent = 100/dataRGs.length;
      for(var i = 0;i<(desDataGroupName.length*sourceDataGroupName.length) ;i++)
      {
         CL.split(sourceDataGroupName[i%sourceDataGroupName.length],desDataGroupName[i%desDataGroupName.length],percent) ;
/*
         var count = db.listTasks().toArray() ;
         var cnt = 0 ;
         while( count.length != 0 )
         {
            count = db.listTasks().toArray() ;
            cnt++ ;
            println( "loop " + cnt + ", task " + db.listTasks() ) ;
         }
*/
         println("old percent is "+percent) ;
         percent = 100*percent/(100-percent) ;
         println("new percent is "+percent) ;
      }
   }
   catch(e)
   {
      throw e;
   }
   try
   {
      println( db.snapshot(8) ) ;
   }
   catch(e)
   {
      throw e;
   }
   try
   {
      db.dropCS(CSPREFIX_CS) ;
      println("dropCS sucessfully!! in the end") ;
   }
   catch(e)
   {
      println("Cann't drop the CS : "+CSPREFIX_CS+" error code is:"+e) ;
      throw e;
   }
   //return 0;
}

// Run Main
try
{
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   main() ;
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}

