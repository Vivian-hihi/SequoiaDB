package com.sequoiadb.cappedCL;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* FileName: InsertCommonCLAndCappedCL11782.java
* test content:test concurrentcy insert for commonCL and cappedCL 
* @author liuxiaoxuan
    * @Date    2017.7.18
*/

public class InsertCommonCLAndCappedCL11782 extends SdbTestBase{

	private Sequoiadb sdb = null;
	private DBCollection cappedCL_11782 = null;
	private DBCollection commonCL_11782 = null;
	private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	private String cappedCSName_11782 = "story_java_cappedCS_11782";
	private String commonCSName_11782 = "story_java_commonCS_11782";
	private String cappedClName_11782 = "cappedCL_11782";
	private String commonClName_11782 = "commonCL_11782";
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
		try {
			boolean isCapped = true;
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
			sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
			cappedCL_11782 = Commlib.createCL(sdb, cappedCSName_11782, cappedClName_11782 ,isCapped);
			commonCL_11782 = Commlib.createCL(sdb, commonCSName_11782, commonClName_11782 ,false);
		}catch(BaseException e) {
			System.out.println("Error message: " + e.getMessage());
			Assert.fail(e.getMessage());
		}
	}
	
	@Test
	public void testGreatConcurrencyInsert(){
		StringBuffer strBuffer =  new StringBuffer();
		
		InsertThread insertThread = new InsertThread(strBuffer);
		
		int threadNum = 10;
		insertThread.start(threadNum);
		
		Assert.assertTrue(insertThread.isSuccess(),insertThread.getErrorMsg());
		Assert.assertTrue(Commlib.checkLogicalID(cappedCL_11782, strBuffer.length(),this.getClass().getName()));
	}
	
	@AfterClass
	public void tearDown() {
		try {
			CollectionSpace cappedCS = sdb.getCollectionSpace(cappedCSName_11782);
			CollectionSpace commonCS = sdb.getCollectionSpace(commonCSName_11782);
			if(cappedCS != null ) {
				sdb.dropCollectionSpace(cappedCSName_11782);	
			}
			if( commonCS != null) {
				sdb.dropCollectionSpace(commonCSName_11782);
			}
		}catch (BaseException e) {
			System.out.println("teardown exception 11782: " + e.getMessage());
			Assert.fail(e.getMessage());
		}finally {
			sdb.close();
			System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
		}
	}
	
	private class InsertThread extends SdbThreadBase {
		
		StringBuffer strBuffer = null;
        BSONObject obj = null;
    	
    	public InsertThread(StringBuffer strBuffer) {
    		this.obj = new BasicBSONObject();
    		this.strBuffer = strBuffer;
    		int stringLength = Commlib.getRandomStringLength();
    	    for(int len = 0; len < stringLength; len++) {    
    	    	strBuffer.append("a");
    	    }
    	    obj.put("a", strBuffer.toString()); 
    	}
		
    	@Override
        public void exec() throws BaseException{
            Sequoiadb db = null;
            DBCollection capCl = null;
            DBCollection commCl = null;
            try{
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
                capCl = db.getCollectionSpace(cappedCSName_11782).getCollection(cappedClName_11782);
                commCl = db.getCollectionSpace(commonCSName_11782).getCollection(commonClName_11782);
                //insert records in cappedCL and commonCL
                Commlib.insertRecords(capCl,obj);
                Commlib.insertRecords(commCl,obj);
            }catch(BaseException e){
                if(e.getErrorCode() != -23 || e.getErrorCode() != -34){
                    throw e;
                }
            }finally{
                db.close();
            }
        }
	}
}
