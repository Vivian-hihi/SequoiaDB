package com.sequoiadb.crud.numoverflow;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* FileName: UpdateUseInc12610.java
* test content:Numeric value overflow for single character using $inc operation,
* 				and the $inc is used as a update.
* testlink case:seqDB-12610
* @author luweikang
    * @Date    2017.9.13
* @version 1.00
*/

public class UpdateUseInc12610 extends SdbTestBase{
	
	private String clName = "inc_update12610";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private static DBCollection cl = null; 
	
	
	@DataProvider(name = "operData")
	public Object[][] generateIntDatas(){
		String []expRecords1 = {"{'no':-2147493648,'long':{'$numberLong':'-9223372036854775808'},'test':1}"};
		String []expRecords2 = {"{'no':-2147493648,'long':{'$decimal':'-18446744073709551616'},'test':1}"};
		String []expRecords3 = {"{'no':{a:{b:2147493648}},'string':'123','test':2}"};
//		String []expRecords4 = {"{'no':[-2147483648,[3221225471],{'$numberLong':'9223372036854775807'}],test:3}"};
//		String []expRecords5 = {"{'no':[-2147483648,[3221225471],{'$decimal':'9223372036854775808'}],test:3}"};
		
		String expJavaLong = "class java.lang.Long";
		String expJavaDecimal = "class org.bson.types.BSONDecimal";
		String expLongType = "int64";
		String expDecimalType = "decimal"; 
		
		return new Object[][]{
			//the parameters: int matcherValue,String updateName, String updateValue, String []expRecords,String expTypeToSdb,Boolean isVerifyTypeToJava, String expTypeToJava
			new Object[]{1,"no",new Integer(-10000),expRecords1,expLongType,true,expJavaLong},
			new Object[]{1,"long",new Long(-9223372036854775808L),expRecords2,expDecimalType,true,expJavaDecimal},
			new Object[]{2,"no.a.b",new Integer(1073751824),expRecords3,expLongType,false,null},
			
			//SEQUOIADBMAINSTREAM-2795
//			new Object[]{3,"no.1.0",new Integer(1073741824),expRecords4,expLongType,false,null},
//			new Object[]{3,"no.2",new Long(1),expRecords5,expDecimalType,false,null},
//			
		};
	}
	
	@BeforeClass
	public void setUp(){
		System.out.println(this.getClass().getName()+" begin at "
					+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+coordUrl+e.getMessage());
		}
		
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		cl = Commlib.createCL(cs, clName);
		String []records = {"{'no':-2147483648,'long':{'$numberLong':'-9223372036854775808'},'test':1}",
		        			"{'no':{a:{b:1073741824}},'string':'123','test':2}",
		        			"{'no':[-2147483648,[2147483647],{'$numberLong':'9223372036854775807'}],test:3}"};

		Commlib.insert(cl, records);
	}
	
	@Test(dataProvider = "operData")
	public void testInc(int matcherValue,String updateName, Object updateValue, String []expRecords,String expTypeToSdb,Boolean isVerifyTypeToJava, String expTypeToJava){
		try{			
			BSONObject uValue = new BasicBSONObject();
			uValue.put(updateName, updateValue);
			Commlib.updateOper(cl, matcherValue, uValue, "update"); 
			Commlib.checkUpdateResult(cl, matcherValue, expRecords);
			try {
				Commlib.checkUpdateDataType(cl, matcherValue, updateName, expTypeToSdb, isVerifyTypeToJava, expTypeToJava);
			} catch (Exception e) {
				e.printStackTrace();
			}
			
		}catch(BaseException e){			
			Assert.assertTrue(false,"inc data is used as update oper failed,"+e.getMessage());
		}		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			System.out.println(this.getClass().getName()+" end at "
					 +new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));			
			CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
			if(cs.isCollectionExist(clName)){
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
}
