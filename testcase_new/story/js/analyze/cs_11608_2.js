/************************************
*@Description: 指定cs清空统计信息,覆盖：
               cs不存在、
               系统cs、
               cs下无cl、
               cs下有cl有索引无数据、
               cs下有cl有数据无索引、
               cs下有cl无数据无索引
*@author:      zhaoyu
*@createdate:  2017.11.9
*@testlinkCase:seqDB-11608
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11608";
   var clName = COMMCLNAME + "_11608";
   var insertNum = 2000;
   
   //清理环境
   commDropCL( db, csName, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, csName, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   
   //执行统计
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   println("check cl:" + clName + " no data success!");
   
   //删除索引，插入数据，执行统计
   commDropIndex( dbcl, "a" );
   insertDatas( dbcl, insertNum );
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, false );
   println("check cl:" + clName + " no index success!");
   
   //删除数据，执行统计
   dbcl.truncate();
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   println("check cl:" + clName + " no index and no data success!");
   
   //删除cl，执行统计
   commDropCL( db, COMMCSNAME, clName) ;
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   println("check cl:" + csName + " no cl success!");
   
   //删除cs，执行统计
   /*
   commDropCS( db, csName );
   try
   {
      db.analyze({CollectionSpace:csName});
      throw "NEED_ERR";
   }catch(e)
   {
      if(e !== -34)
      {
         throw e;
      }
   }
   
   //指定系统表执行统计
   try
   {
      db.analyze({CollectionSpace:"SYSSTAT"});
      throw "NEED_ERR";
   }catch(e)
   {
      if(e !== -34)
      {
         throw e;
      }
   }
   */ 
 }
 main()