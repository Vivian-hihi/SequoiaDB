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
* FileName: AdsIsSelector12570.java
* test content:Numeric value overflow for single character using $abs operation,
* 				and the $abs is used as a selector.
* testlink case:seqDB-12570
* @author wuyan
    * @Date    2017.9.1
* @version 1.00
*/

public class AdsIsSelector12570 extends SdbTestBase{	
	
	private String clName = "abs_selector12570";
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
		
		String clOption = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
				+ "ReplSize:0,Compressed:true, StrictDataMode:false}";
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		cl = Commlib.createCL(cs,  clName, clOption);	
		
		String []records = {"{'no':-2147483648,'tlong':{'$numberLong':'-9223372036854775808'},'tdouble':-1.7E+308,'test':0}",
		        "{no:1,numarry:[-2147483648,{'$numberLong':'-9223372036854775808'},'testo1'],obj:{a:-2147483648},test:1}",
		        "{no:2147483647,obj:{a:{b:{'$numberLong':'-9223372036854775808'}}},test:3}"};

		Commlib.insert(cl, records);
	}
	
	@Test
	public void testAds(){
		//test int32 type numberflow
		String expType = "class java.lang.Long";
		String []expRecords = {"{'no':2147483648,'tlong':-9223372036854775808,'tdouble':-1.7E+308,'test':0}"}; 
		int mValue = 0;
		String sName = "no";
		adsAsSelectorOper(mValue, sName, expRecords,expType, true);
		getDataType("$abs",mValue,sName, "");
		
		//test int64 type numberflow
		String expType1 = "class org.bson.types.BSONDecimal";		
		String sName1 = "tlong";
		String []expRecords1 = {"{'no':-2147483648,'tlong':{ '$decimal' : '9223372036854775808'},'tdouble':-1.7E+308,'test':0}"};
		adsAsSelectorOper(mValue, sName1, expRecords1,expType1, true);
		
		//test double type use $abs 
		String expType2 = "class java.lang.Double";		
		String sName2 = "tdouble";
		String []expRecords2 = {"{'no':-2147483648,'tlong':-9223372036854775808,'tdouble':1.7E+308,'test':0}"};
		adsAsSelectorOper(mValue, sName2, expRecords2,expType2, true);
		
		//test array element use $abs
		String expType3 = "";		
		String sName3 = "numarry.$[0]";
		int mValue3 = 1;
		String []expRecords3 = {"{no:1,numarry:[2147483648],obj:{a:-2147483648},test:1}"};
		adsAsSelectorOper(mValue3, sName3, expRecords3,expType3, false);
		
		//test array all element use $abs		
		String sName4 = "numarry";
		int mValue4 = 1;
		String []expRecords4 = {"{no:1,numarry:[2147483648,{ '$decimal' : '9223372036854775808'} , null],obj:{a:-2147483648},test:1}"};
		adsAsSelectorOper(mValue4, sName4, expRecords4, null, false);
		
		//test obj type use $abs
		String expType4 ="";
		String sName5 = "obj.a.b";
		int mValue5 = 3;
		String []expRecords5 = {"{no:2147483647,obj:{a:{b:{'$decimal':'9223372036854775808'}}},test:3}"};
		adsAsSelectorOper(mValue5, sName5, expRecords5,expType4, false);
		//getDataType("$abs",mValue5,sName5, "");
		
	}
	
	
		
	@AfterClass
	public void tearDown(){
		try{
			System.out.println(this.getClass().getName()+" end at "
					 +new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));		
			
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}			
		}catch(BaseException e){
			Assert.fail("clear env failed, errMsg:" + e.getMessage());
		}finally {
			if (sdb != null){
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
	
	public static void getDataType(String opeSymbol,int matcherValue,String selectorName, String expType){
		try{
			DBQuery query = new DBQuery();
			BSONObject selector = new BasicBSONObject();
			BSONObject sValue = new BasicBSONObject();
			BSONObject sValue1 = new BasicBSONObject();
			BSONObject matcher = new BasicBSONObject();					
			//matcher.put("test",0);
			matcher.put("test",matcherValue );
			sValue.put(opeSymbol, 1);			
			sValue1.put("$type",2);
			//selector.put("no", sValue);
			selector.put(selectorName, sValue);	
			selector.put(selectorName, sValue1);	
			query.setSelector(selector);
			query.setMatcher(matcher);		
			DBCursor cursor = cl.query(query);
			List<BSONObject> actualList= new ArrayList<BSONObject>(); 
			
			//String expType = "class java.lang.Long";
			String type = "";			
	        while( cursor.hasNext() ) {
	        	System.out.println("---begin2");
	            BSONObject object = cursor.getNext();	
	            System.out.println("slectoryName="+selectorName);
	            type = object.get(selectorName).toString();	            
	            System.out.println("objecttype=="+type);
	            actualList.add(object);
	        }  
	        System.out.println("actualList"+actualList.toString());
	        cursor.close(); 	        
	        
		}catch(BaseException e){			
			Assert.assertTrue(false,"abs is used as selector oper failed,"+e.getMessage());
		}		
	}


}
