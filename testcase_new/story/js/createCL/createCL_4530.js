/************************************
*@Description:  createCL，覆盖name有效字符和边界_st.verify.CL.001
*@author:      wangkexin
*@createDate:  2019.6.6
*@testlinkCase: seqDB-4530
**************************************/
main();
function main()
{
   if(true == commIsStandalone( db ))
   {
      println( "run mode is standalone");
      return;
   }
   
   var groupsArray = commGetGroups( db, false, "", true, true, true );
   var groupName = groupsArray[0][0].GroupName;
   
   var csName = COMMCSNAME;
   var clNames = ["a","123-test-中文。~!@#%^&*()_+-=\][|,/<>?~·！@#￥%……&*~!-%^*()_-+=|\/<'?*[];:/#（）——+《》{}|【】、，。~_127B","testIsMainCL"];
   
   for(var i = 0 ; i < clNames.length; i++)
   {
       commDropCL( db, csName, clNames[i], true, true, "drop cl " + clNames[i] + " in the beginning." );
   }

   //name 覆盖1字节有效字符
   var optionObj = {ShardingKey:{a:1},ShardingType:"hash",Partition:1024,ReplSize:0,Compressed:true,CompressionType:"lzw",AutoSplit:true};
   commCreateCLByOption( db, csName, clNames[0], optionObj, true );
   
   checkResult( clNames[0], "ShardingKey", {a:1} );
   checkResult( clNames[0], "ShardingType", "hash" );
   checkResult( clNames[0], "Partition", 1024 );
   checkResult( clNames[0], "ReplSize", 7 );
   checkResult( clNames[0], "AttributeDesc", "Compressed" );
   checkResult( clNames[0], "CompressionTypeDesc", "lzw" );
   checkResult( clNames[0], "AutoSplit", true );
   
   //name 覆盖127字节有效字符
   var optionObj = {ShardingKey:{a:1},ShardingType:"range",ReplSize:-1,Group:groupName};
   commCreateCLByOption( db, csName, clNames[1], optionObj, true );
   
   checkResult( clNames[1], "ShardingKey", {a:1} );
   checkResult( clNames[1], "ShardingType", "range" );
   checkResult( clNames[1], "ReplSize", -1 );
   checkGroup( clNames[1], groupName );
   
   //检测 IsMainCL
   var optionObj = {ShardingKey:{a:1},ShardingType:"range",IsMainCL:true};
   commCreateCLByOption( db, csName, clNames[2], optionObj, true );
   
   checkResult( clNames[2], "ShardingKey", {a:1} );
   checkResult( clNames[2], "ShardingType", "range" );
   checkResult( clNames[2], "IsMainCL", true );
   
   for(var i = 0 ; i < clNames.length; i++)
   {
       commDropCL( db, csName, clNames[i], true, true, "drop cl " + clNames[i] + " in the end." );
   }
}

function checkResult(clName, fieldName, expFieldValue)
{
   var clFullName = COMMCSNAME + "." + clName;   
   var cur = db.snapshot(8,{"Name":clFullName});
   var actualFieldValue = cur.current().toObj()[fieldName];
   
   if ( typeof(expFieldValue) === "object" )
   {      
      if (JSON.stringify(expFieldValue) !== JSON.stringify(actualFieldValue))
      {
         throw buildException("test fieldvalue1", "check field", "value is wrong", JSON.stringify(expFieldValue), JSON.stringify(actualFieldValue));
      }
      
   }
   else
   {
      if (expFieldValue  !== actualFieldValue)
      {
         throw buildException("test fieldvalue2", "check field", "value is wrong", expFieldValue, actualFieldValue);
      }
   }
   
}

function checkGroup( clName, groupName )
{
   var actGroups = [];
   var expGroups = [groupName]
   var clFullName = COMMCSNAME + "." + clName;  
   var cur = db.snapshot(4,{"Name":clFullName});
   if( cur.next() )
   {
       var groups = cur.current().toObj()["Details"];
       for(var i = 0 ; i < groups.length; i++)
       {
           actGroups.push(groups[i].GroupName);
       }
   }
   
   if (JSON.stringify(expGroups) !== JSON.stringify(actGroups))
   {
      throw buildException("checkGroup()", null, "groups is wrong", JSON.stringify(expGroups), JSON.stringify(actGroups));
   }
}