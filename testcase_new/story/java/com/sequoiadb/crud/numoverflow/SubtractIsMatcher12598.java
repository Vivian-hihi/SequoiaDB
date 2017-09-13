package com.sequoiadb.crud.numoverflow;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.crud.numoverflow.Commlib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* FileName: SubtractIsSelector12573.java
* test content:Numeric value overflow for single character using $subtract operation,
* 				and the $subtract is used as a selector.
* testlink case:seqDB-12573
* @author wuyan
    * @Date    2017.9.4
* @version 1.00
*/

public class SubtractIsMatcher12598 extends SdbTestBase{
	
	
	private String clName = "subtract_selector12573";
	private CollectionSpace cs = null;
	private Sequoiadb sdb = null;
	private static DBCollection cl = null;    
	
	@BeforeClass
	public void setUp(){
		System.out.println(this.getClass().getName()+" begin at "
					+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+coordUrl+e.getMessage());
		}
		
		String clOption = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
				+ "ReplSize:0,Compressed:true, IsStrictDataType:false}";
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		cl = Commlib.createCL(cs, clName, clOption);
		
		String []records = {"{'no':-2147483648,'tlong':{'$numberLong':'-9223372036854775808'},'test':0}",
		        "{no:2147483647,'long':{'$numberLong':'9223372036854775807'},test:1}",
		        "{no:1147483147,'long':{'$numberLong':'-8223372036854775807'},arr:[1000000000,-2100],test:2}",
		        "{no:[2147483147,{'$numberLong':'8223372036854775807'}],arr:[1000000000,-2147483648],test:3}"};

		Commlib.insert(cl, records);
	}
	
	@DataProvider(name = "operIntTypeData")
	public Object[][] generateIntDatas(){
		String []expRecords1 = {"{'no':-2147483649,'tlong':{'$numberLong':'-9223372036854775808'},'test':0}"};		
		String []expRecords2 = {"{no:2147483648,'long':{'$numberLong':'9223372036854775807'},test:1}"};	
		String []expRecords3 = {"{no:2194966294,'long':{'$numberLong':'-8223372036854775807'},arr:[1000000000,-2100],test:2}"};
		String []expRecords4 = {"{no:1147483147,long:-8223372036854775807,arr:[ -1147481549,-2147483649],test:2}}"};		
		String expLongType = "class java.lang.Long";		
		return new Object[][]{
			//the parameters: matcherValue,subValue,selectorName,expRecords,expType,isVerifyDataType
			//-2147483648 $substract 1,the result is -2147483649(int64)
			new Object[]{0, 1, "no",expRecords1,expLongType,true},			
			//2147483647 $substract -1,the result is 2147483648(int64)
			new Object[]{1, -1, "no",expRecords2,expLongType,true},			
			//1147483147 $substract -1047483147,the result is 2194966294(int64)
			new Object[]{2, -1047483147, "no",expRecords3,expLongType,true},	
			//the arr.1 -2100 $substract 2147481549,the result is -2147483649(int64)
			new Object[]{2, 2147481549, "arr",expRecords4,null,false},	
		};
	}
	
	
	//@Test(dataProvider = "operIntTypeData")
	public void subtractAsSelectorOper(int matcherValue,int subValue,String selectorName, String []expRecords,String expType,Boolean isVerifyDataType){
		try{			
			DBQuery query = new DBQuery();
			BSONObject selector = new BasicBSONObject();
			BSONObject sValue = new BasicBSONObject();
			BSONObject sValue1 = new BasicBSONObject();
			BSONObject matcher = new BasicBSONObject();				
			matcher.put("test",matcherValue );
			sValue.put("$subtract", subValue);
			sValue1.put("$include",0);	
			selector.put(selectorName, sValue);
			selector.put("_id", sValue1);
			query.setSelector(selector);
			query.setMatcher(matcher);		
			DBCursor cursor = cl.query(query);
			List<BSONObject> actualList= new ArrayList<BSONObject>(); 
			
			//String expType = "class java.lang.Long";
	        while( cursor.hasNext() ) {
	            BSONObject object = cursor.getNext();
	            System.out.println("object"+object.toString());
	            actualList.add(object);
	        }  
	        System.out.println("actualList"+actualList.toString());
	        cursor.close(); 
	        	        
	        List<BSONObject> expectedList= new ArrayList<BSONObject>();
			for (int i = 0; i < expRecords.length; i++) {	
				BSONObject expRecord =(BSONObject) JSON.parse(expRecords[i]);
				expectedList.add(expRecord);                
            }			
			Assert.assertEquals(actualList, expectedList,"the actual query datas is error");
	        	        
	        if(isVerifyDataType){
	        	String type = "";
	        	DBCursor cursor1 = cl.query(query);
	        	while( cursor1.hasNext() ) {
		            BSONObject object1 = cursor1.getNext();
		            System.out.println("slectoryName="+selectorName);
		            type = object1.get(selectorName).getClass().toString();	            
		            System.out.println("objecttype"+object1.get(selectorName).getClass().toString());	
		           /* BasicBSONObject type1 = (BasicBSONObject) object.get("obj");
		            BasicBSONObject type2 = (BasicBSONObject) type1.get("a");
		            System.out.println("objecttype"+type2.get("b").getClass());*/		           
		        }  		        
		        cursor1.close(); 
		        Assert.assertEquals(type, expType,"the numtype is error");
	        }
	        		
		}catch(BaseException e){			
			Assert.assertTrue(false,"subtract is used as selector oper failed,"+e.getMessage());
		}		
	}
	
	@DataProvider(name = "operLongTypeData")
	public Object[][] generateLongDatas(){
		String []expRecords1 = {"{'no':-2147483648,'tlong':{'$decimal':'-9223372036854775809'},'test':0}"};
		String []expRecords2 = {"{'no':2147483647,'tlong':{'$decimal':'-9223372036854775809'},'test':1}"};	
		String []expRecords3 = {"{no:1147483147,'long':{'$decimal':'-9223372036854775809'},"
									+ "arr:[1000000000,-2100],test:2}"};
		String []expRecords4 = {"{no:1147483147,'long':{'$numberLong':'-8223372036854775807'},"
									+ "arr:[{'$decimal':'9223372036854775808'},{'$numberLong':'9223372035854776018l'}],test:2}"};
		String expDecimalType = "class org.bson.types.BSONDecimal";
		return new Object[][]{
			//the parameters: matcherValue,subValue,selectorName,expRecords,expType,isVerifyDataType
			//-9223372036854775808 $substract 1,the result is {'$decimal':'-9223372036854775809'}
			new Object[]{0, 1l, "tlong",expRecords1,expDecimalType,true},
			//9223372036854775807 $substract -2,the result is 2147483648(int64)
			new Object[]{1, -2l, "tlong",expRecords2,expDecimalType,true},
			//-8223372036854775807 $substract -1000000000000000002,the result is {'$decimal':'9223372036854775809'}
			new Object[]{2, -1000000000000000002l, "tlong",expRecords3,expDecimalType,true},
			//arr:[1000000000,-2147483648] $substract -8223372036854775809
			new Object[]{2, -9223372035854775808l, "arr",expRecords4,null,false},
			//-8223372036854775807 $substract -1000000000000000002,the result is {'$decimal':'9223372036854775809'}
			//new Object[]{2, "-1000000000000000002", "arr.1",expRecords4,expLongType,true},
			
		};
	}
	@Test(dataProvider = "operLongTypeData")
	public static void subtractLongData(int matcherValue,long subValue,String selectorName, String []expRecords,String expType,Boolean isVerifyDataType){
		try{			
			DBQuery query = new DBQuery();
			BSONObject selector = new BasicBSONObject();
			BSONObject sValue = new BasicBSONObject();
			BSONObject sValue1 = new BasicBSONObject();
			BSONObject matcher = new BasicBSONObject();				
			matcher.put("test",matcherValue );
			sValue.put("$subtract", subValue);
			sValue1.put("$include",0);	
			selector.put(selectorName, sValue);
			selector.put("_id", sValue1);
			query.setSelector(selector);
			query.setMatcher(matcher);		
			DBCursor cursor = cl.query(query);
			List<BSONObject> actualList= new ArrayList<BSONObject>(); 
			
			//String expType = "class java.lang.Long";
	        while( cursor.hasNext() ) {
	            BSONObject object = cursor.getNext();
	            System.out.println("object"+object.toString());
	            actualList.add(object);
	        }  
	        System.out.println("actualList"+actualList.toString());
	        cursor.close(); 
	        	        
	        List<BSONObject> expectedList= new ArrayList<BSONObject>();
			for (int i = 0; i < expRecords.length; i++) {	
				BSONObject expRecord =(BSONObject) JSON.parse(expRecords[i]);
				expectedList.add(expRecord);                
            }			
			Assert.assertEquals(actualList, expectedList,"the actual query datas is error");
	        	        
	        if(isVerifyDataType){
	        	String type = "";
	        	DBCursor cursor1 = cl.query(query);
	        	while( cursor1.hasNext() ) {
		            BSONObject object1 = cursor1.getNext();
		            System.out.println("slectoryName="+selectorName);
		            type = object1.get(selectorName).getClass().toString();	            
		            System.out.println("objecttype"+object1.get(selectorName).getClass().toString());	
		           /* BasicBSONObject type1 = (BasicBSONObject) object.get("obj");
		            BasicBSONObject type2 = (BasicBSONObject) type1.get("a");
		            System.out.println("objecttype"+type2.get("b").getClass());*/		           
		        }  		        
		        cursor1.close(); 
		        Assert.assertEquals(type, expType,"the numtype is error");
	        }
	        		
		}catch(BaseException e){			
			Assert.assertTrue(false,"subtract is used as selector oper failed,"+e.getMessage());
		}		
	}

	
	
		
	@AfterClass
	public void tearDown(){
		try{
			System.out.println(this.getClass().getName()+" end at "
					 +new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));			
			CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
			if(cs.isCollectionExist(clName)){
				//cs.dropCollection(clName);
			}
			sdb.close();
		}catch(BaseException e){
			Assert.fail("clear env failed, errMsg:" + e.getMessage());
		}
	}
	
		
	
	
	
}
