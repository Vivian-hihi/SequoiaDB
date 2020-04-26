/******************************************************************************
*@Description : seqDB-11372:匹配组合索引的索引选择
*@author      : Li Yuanyue
*@Date        : 2020.4.25
******************************************************************************/
testConf.clName = CHANGEDPREFIX + "_11372";

main( test );

function test ()
{
   var dataGroupNames = commGetDataGroupNames( db );
   var csName = COMMCSNAME;
   var clName = CHANGEDPREFIX + "_11372";
   var idxName1 = "index_ab_11372";
   var idxName2 = "index_bc_11372";

   commDropCL( db, csName, clName );

   var cl = commCreateCL( db, csName, clName, { Group: dataGroupNames[0] }, false );

   cl.createIndex( idxName1, { a: 1, b: 1, c: 1 } );
   cl.createIndex( idxName2, { b: -1, c: -1 } );

   var arr = [10, 12, 8, 13, 7, 10, 10, 10, 10, 10]
   for( var i = 0; i < arr.length; i++ )
   {
      for( var j = 0; j < arr[i]; j++ )
      {
         cl.insert( { a: i + 1, b: i + 1, c: i + 1, d: i + 1 } );
      }
   }

   db.analyze( { Collection: csName + "." + clName } );

   var findCond = { "b": 5, "c": 5 };
   var cur = cl.find( findCond ).explain( { Evaluate: true } );
   var expIndex = idxName2;
   checkOptimalIndex( cur, expIndex );

   var findCond = { "a": 9, "c": 9 };
   var cur = cl.find( findCond ).explain( { Evaluate: true } );
   var expIndex = idxName1;
   checkOptimalIndex( cur, expIndex );

   commDropCL( db, csName, clName, false );

}
function checkOptimalIndex ( cur, expIndex )
{
   var searchPaths = cur.current().toObj().PlanPath.ChildOperators[0].Search.SearchPaths;
   for( var i = 0; i < searchPaths.length; i++ )
   {
      var actIndex = searchPaths[i].IndexName;
      var isUsed = searchPaths[i].IsUsed;
      if( isUsed === true )
      {
         if( expIndex !== actIndex )
         {
            throw new Error( "optimal Index error,expIndex:" + expIndex + ",but actually actIndex:" + actIndex );
         }
      }
   }
}