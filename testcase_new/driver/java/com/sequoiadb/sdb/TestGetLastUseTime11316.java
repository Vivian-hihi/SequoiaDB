package com.sequoiadb.sdb;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* FileName: TestGetLastUseTime11316
* test interface:getLastUseTime()* 
* @author wuyan
    * @Date    2017.4.7
* @version 1.00
*/
public class TestGetLastUseTime11316 extends SdbTestBase{	
	private String clName = "cl_11316";
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	
	@BeforeClass
	public void setUp( ){
		System.out.println(this.getClass().getName()+" begin at "
				+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			sdb.setSessionAttr(new BasicBSONObject("PreferedInstance", "M")); 
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+SdbTestBase.coordUrl+e.getMessage());
		}		
	}
	
	@Test
	public void testLastUseTime(){
		try{
			long beforeTime = sdb.getLastUseTime();		
			createCL();		
			long afterTime = sdb.getLastUseTime();
	
			if (beforeTime >= afterTime)
			{
				Assert.assertTrue(false,"beforeTime greater than afterTime!"+"  beforeTime:"
			                         +beforeTime+"   afterTime:"+afterTime);
			}
		}catch(BaseException e){		   
		   Assert.assertTrue(false, e.getErrorCode()+e.getMessage());	
		}
	}	
	
	@AfterClass()
	public void tearDown(){
		try{
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}
			sdb.close();
			System.out.println(this.getClass().getName()+" end at "
					+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		}catch(BaseException e){			
			Assert.assertTrue(false,"clean up failed:"+e.getMessage());
		}
	}	
	
	private void createCL(){
		try{
			if (!sdb.isCollectionSpaceExist(SdbTestBase.csName)){
				sdb.createCollectionSpace(SdbTestBase.csName);	
			}
		}catch(BaseException e){
			//-33 CS exist,ignore exceptions
			Assert.assertEquals(-33,e.getErrorCode(),e.getMessage());
		}
		
		try
		{
			cs = sdb.getCollectionSpace(SdbTestBase.csName);			
			cs.createCollection(clName);
		}catch(BaseException e){
			Assert.assertTrue(false,"create cl fail "+e.getErrorType()+":"+e.getMessage());
		}
	}
	
	
}
