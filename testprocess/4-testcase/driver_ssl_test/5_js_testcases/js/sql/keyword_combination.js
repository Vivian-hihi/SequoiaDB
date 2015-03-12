db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME);
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL1 = CSPREFIX+"product" ;
CSPREFIX_CL2 = CSPREFIX+"name" ;
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}
try
{
	 db.createCS(CSPREFIX_CS);
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.getCS(CSPREFIX_CS);
   var cl_pro = varCS.createCL(CSPREFIX_CL1,{ReplSize:claSize.ReplSize()});
   var cl_name = varCS.createCL(CSPREFIX_CL2,{ReplSize:claSize.ReplSize()});
}
catch(e)
{
	throw e;
}

try
{
   cl_pro.insert({
    "_id": {
    "$oid": "5267297d4e8d54b12c000003"
  },
  "other1": 1,
  "other2": "xxx",
  "other3": [
    1,
    2,
    3
  ],
  "productCategoryList": [
    {
      "cateID": 1,
      "cateNum": 1,
      "employType": "1",
      "categoryInfoList": [
        {
          "content": "cateID1_1",
          "categroyNum": 1
        },
        {
          "content": "cateID1_2",
          "categroyNum": 1
        }
      ]
    },
    {
      "cateID": 2,
      "cateNum": 1,
      "employType": "1",
      "categoryInfoList": [
        {
          "content": "cateID2_1",
          "categroyNum": 1
        },
        {
          "content": "cateID2_2",
          "categroyNum": 1
        }
      ]
    }
  ]
});
}
catch(e)
{
   throw e;
}

try
{
cl_pro.insert(
{
  "_id": {
    "$oid": "5267430f4e8d54b12c000004"
  },
  "other1": 2,
  "other2": "xxx",
  "other3": [
    1,
    2,
    3
  ],
  "productCategoryList": [
    {
      "cateID": 1,
      "cateNum": 1,
      "employType": "1",
      "categoryInfoList": [
        {
          "content": "cateID1_1",
          "categroyNum": 1
        },
        {
          "content": "cateID1_2",
          "categroyNum": 1
        }
      ]
    },
    {
      "cateID": 2,
      "cateNum": 1,
      "employType": "1",
      "categoryInfoList": [
        {
          "content": "cateID2_1",
          "categroyNum": 1
        },
        {
          "content": "cateID2_2",
          "categroyNum": 1
        }
      ]
    }
  ]
});
}
catch(e)
{
   throw e;
}

try
{
cl_name.insert({
  "_id": {
    "$oid": "526728334e8d54b12c000000"
  },
  "cateID": 1,
  "cateName": "name1"
});
}
catch(e)
{
   throw e;
}

try
{
cl_name.insert({
  "_id": {
    "$oid": "526728384e8d54b12c000001"
  },
  "cateID": 2,
  "cateName": "name2"
});
}
catch(e)
{
   throw e;
}

try
{
cl_name.insert({
  "_id": {
    "$oid": "5267283d4e8d54b12c000002"
  },
  "cateID": 3,
  "cateName": "name3"
});
}
catch(e)
{
   throw e;
}

try
{
	 var sqlstr= "select T4.other1, T4.other2, T4.other3, push(T4.productCategoryList) as productCategoryList from (select T2.other1, T2.other2, T2.other3, buildobj(T2.cateNum, T2.employType, T2.categoryInfoList, T3.cateName) as productCategoryList from ( select T1.other1, T1.other2, T1.other3, T1.productCategoryList.cateID as cateID, T1.productCategoryList.cateNum as cateNum, T1.productCategoryList.employType as employType, T1.productCategoryList.categoryInfoList as categoryInfoList from ( select * from ";
	 sqlstr += CSPREFIX_CS;
	 sqlstr += ".";
	 sqlstr += CSPREFIX_CL1;
	 sqlstr += " split by productCategoryList ) as T1  ) as T2 inner join "
	 sqlstr += CSPREFIX_CS;
	 sqlstr += ".";
	 sqlstr += CSPREFIX_CL2;
	 
	 sqlstr += " as T3 on T2.cateID = T3.cateID) as T4 group by T4.other1, T4.other2, T4.other3"
	 println(sqlstr);
   //var cur = db.exec(sqlstr/*"select T4.other1, T4.other2, T4.other3, push(T4.productCategoryList) as productCategoryList from (select T2.other1, T2.other2, T2.other3, buildobj(T2.cateNum, T2.employType, T2.categoryInfoList, T3.cateName) as productCategoryList from ( select T1.other1, T1.other2, T1.other3, T1.productCategoryList.cateID as cateID, T1.productCategoryList.cateNum as cateNum, T1.productCategoryList.employType as employType, T1.productCategoryList.categoryInfoList as categoryInfoList from ( select * from foo.product split by productCategoryList ) as T1  ) as T2 inner join  foo.name as T3 on T2.cateID = T3.cateID) as T4 group by T4.other1, T4.other2, T4.other3"*/);
   var cur=db.exec(sqlstr);
}
catch(e)
{
   println("combination select failed rc = " + e );
}

if ( 2 != cur.size() )
{
   println( "get record number error,result record num: "+cur.size() ) ;
   throw -8 ;
}

var num = 1;
while( cur.next() ){
      if(num != cur.current().toObj()["other1"] )
         throw -1;	
      num = num + 1;
   }
   
try
{
   db.dropCS(CSPREFIX_CS);
}
catch(e)
{
   throw e;
}
