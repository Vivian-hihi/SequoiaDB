/************************************
*@Description: 索引名取边界值1023字节，验证基本操作
*@author:      wangkexin
*@createdate:  2019.5.28
*@testlinkCase:seqDB-18357
**************************************/
main();
function main()
{      
   var csName = COMMCSNAME;  
   var clName = CHANGEDPREFIX + "_indexcl18357";
   commDropCL( db, csName, clName, true, true, "clear cl in the beginning" );

   var cl = commCreateCL( db, csName, clName, 0, true, true );
   var single_index = "";
   var composite_index = "";
   for(var i = 0; i < 1023; i++)
   {
       single_index += 's';
       composite_index += 'c';
   }
   
   //-------test single index
   cl.createIndex(single_index, {'a':1}, true);

   //crud operation
   var obj = {a:1};
   var newObj = {a:123};
   testInsert( cl, obj );
   var curObj = testUpdate( cl, obj, newObj );
   testQuery(cl, curObj);
   testDelete(cl, curObj);

   //-------test composite index
   cl.createIndex(composite_index, {'b':1, 'c':-1}, true);

   //crud operation
   var obj2 = {b:1, c:"test18357"};
   var newObj2 = {b:456, c:"newtest18357"};
   testInsert( cl, obj2 );
   var curObj2 = testUpdate( cl, obj2, newObj2 );
   testQuery(cl, curObj2);
   testDelete(cl, curObj2);
   
   commDropCL( db, csName, clName, true, true, "clear cl in the end" )
}

function testInsert( cl, obj )
{
   cl.insert(obj);
   var expRecs = [];
   expRecs.push(obj); 
   var rc = cl.find();
   checkRec( rc, expRecs ); 
}

function testUpdate( cl, oldObj, newObj )
{
   var expRecs = [];
   expRecs.push(newObj);
   cl.update({$set:newObj}, oldObj);
   var rc = cl.find();
   checkRec( rc, expRecs ); 
   return newObj;
}

function testQuery( cl, keyValue )
{
   var rc = cl.find(keyValue);
   var expRecs = [];
   expRecs.push(keyValue);
   checkRec( rc, expRecs );
}

function testDelete( cl, keyValue )
{
    cl.remove( keyValue );
    var count = cl.count();
    if( 0 !== Number(count))
    {
        throw buildException( "testDelete", null, "", 0, Number(count) );
    }
}