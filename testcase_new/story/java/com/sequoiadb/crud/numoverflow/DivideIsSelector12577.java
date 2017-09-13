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
* FileName: DivideIsSelector12577.java
* test content:Numeric value overflow for many character using $divide operation,
* 				and the $subtract is used as a selector.
* testlink case:seqDB-12577
* @author wuyan
    * @Date    2017.9.11
* @version 1.00
*/

public class DivideIsSelector12577 extends SdbTestBase{	
	
	private String clName = "divide_selector12577";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
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
		
		String clOption = "{ReplSize:0,Compressed:true, StrictDataMode:false}";
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		cl = Commlib.createCL(cs, clName, clOption);
		
		String []records = {"{'no':-2147483648,'tlong':{'$numberLong':'-9223372036854775808'},"
								+ "'arr':[1,[1,{'$numberLong':'-9223372036854775808'}],2],obj:{a:{b:-2147483648}}}"};
		Commlib.insert(cl, records);
	}
	
	@Test
	public void testDivideAsSelector(){
		try{
			//TODO:SEQUOIADBMAINSTREAM-2795,not supported the arr.$[1].$[1]':{$abs:1}
			//String selector = "{no:{$abs:1},tlong:{$abs:1},"
					//+ "'arr.$[1].$[1]':{$abs:1},'obj.a.b':{$abs:1},_id:{$include:0}}";			
			//String []expRecords = {"{'no':2147483648,'tlong':{'$decimal':'9223372036854775808'},"
	        //		+ "'arr':[1,[1,{'$decimal':'9223372036854775808'}],2],obj:{a:{b:2147483648}}}"};
			String selector = "{no:{$abs:1},tlong:{$abs:1},'obj.a.b':{$abs:1},_id:{$include:0}}";
			String []expRecords = {"{'no':2147483648,'tlong':{'$decimal':'9223372036854775808'},"
					+ "'arr':[1,[1,{'$numberLong':'-9223372036854775808'}],2],obj:{a:{b:2147483648}}}"};			
			Commlib.multipleFieldOper(cl, selector, expRecords);
		}catch(BaseException e){			
			Assert.assertTrue(false,"divide is used as selector oper failed,"+e.getMessage()+e.getErrorCode());
		}		
	}	
	
		
	@AfterClass
	public void tearDown(){
		try{
			System.out.println(this.getClass().getName()+" end at "
					 +new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));			
			if(sdb.getCollectionSpace(SdbTestBase.csName).isCollectionExist(clName)){
				cs.dropCollection(clName);
			}			
		}catch(BaseException e){
			Assert.fail("clear env failed, errMsg:" + e.getMessage());
		}finally{
			if(sdb != null){
				sdb.close();
			}
		}
	}
	
		
	public static void adsAsSelectorOper(int matcherValue,String selectorName, String []expRecords,String expType,Boolean isVerifyDataType){
		try{
			DBQuery query = new DBQuery();
			BSONObject selector = new BasicBSONObject();
			BSONObject sValue = new BasicBSONObject();
			BSONObject sValue1 = new BasicBSONObject();
			BSONObject matcher = new BasicBSONObject();					
			//matcher.put("test",0);
			matcher.put("test",matcherValue );
			sValue.put("$abs", 1);
			sValue1.put("$include",0);
			//selector.put("no", sValue);
			selector.put(selectorName, sValue);
			selector.put("_id", sValue1);
			query.setSelector(selector);
			query.setMatcher(matcher);		
			DBCursor cursor = cl.query(query);
			List<BSONObject> actualList= new ArrayList<BSONObject>(); 
			
			//String expType = "class java.lang.Long";
	        while( cursor.hasNext() ) {
	            BSONObject object = cursor.getNext();	        
	            actualList.add(object);
	        }  
	        System.out.println("actualList"+actualList.toString());
	        cursor.close(); 
	        
	        //String []expRecords = {"{'no':2147483648,'tlong':-9223372036854775808,'tdoulbe':-1.7E+308,'test':0}"}; 
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
			Assert.assertTrue(false,"abs is used as selector oper failed,"+e.getMessage());
		}		
	}

}
