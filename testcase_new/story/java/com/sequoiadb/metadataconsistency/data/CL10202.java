package com.sequoiadb.metadataconsistency.data;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataconsistency.data.MetaDataUtils;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* TestLink: seqDB-10202: concurrency[drop mainCL,dropSubCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10202 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10202";
	private String clName = "cl10202";
	private String mCLName = clName + "_m";
	private String sCLName = clName + "_s";
	
	@BeforeClass
	public void setUp(){
		//start time
		System.out.println("Begin to run " + getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//judge the mode
			if(MetaDataUtils.isStandAlone(sdb)){
				throw new SkipException("The mode is standlone, skip the testCase.");
			}
			MetaDataUtils.clearCS(sdb, csName);
			
			sdb.createCollectionSpace(csName);
			createMainCL(sdb);
			createSubCL(sdb);
			attachCL(sdb);
			MetaDataUtils.insertData(sdb, csName, mCLName);
		}catch(BaseException e){
			sdb.disconnect();
			Assert.fail(e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			MetaDataUtils.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}finally{
			System.out.println("End to run " + getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void test(){
		
		DropMainCL dropMainCL = new DropMainCL();
		dropMainCL.start();
		
		DropSubCL dropSubCL = new DropSubCL();
		dropSubCL.start();
		
		if( !( dropMainCL.isSuccess() && dropSubCL.isSuccess() ) ){
			Assert.fail(dropMainCL.getErrorMsg() + dropSubCL.getErrorMsg());
		}
		
		//check results
		MetaDataUtils.checkCLResult(csName, clName);
	}
	
	private class DropMainCL extends SdbThreadBase{
		@Override
		public void exec() throws BaseException{
			Sequoiadb db = null;
			try
			{
				db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				db.getCollectionSpace(csName).dropCollection(mCLName);
			}catch(BaseException e){
				int eCode = e.getErrorCode();
				if( eCode != -23 && eCode != -147){  //-147:Unable to lock
					throw e;
				}
			}finally{
				db.disconnect();
			}
		}
	}

	private class DropSubCL extends SdbThreadBase{
		@Override
		public void exec() throws BaseException{
			Sequoiadb db  = null;
			try
			{
				db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				CollectionSpace csDB = db.getCollectionSpace(csName);
					
				csDB.dropCollection(sCLName);
			}catch(BaseException e){
				int eCode = e.getErrorCode();
				if( eCode != -23 && eCode != -147){  //-147:Unable to lock
					throw e;
				}
			}finally{
				db.disconnect();
			}
		}
	}
	
	public void createMainCL(Sequoiadb sdb){
		try{
			CollectionSpace csDB = sdb.getCollectionSpace(csName);
			
			BSONObject mOpt = new BasicBSONObject();
			BSONObject mSubObj = new BasicBSONObject();
			mSubObj.put("a", 1);
			mOpt.put("ShardingKey", mSubObj); 
			mOpt.put("ReplSize", 0);
			mOpt.put("IsMainCL", true);
			csDB.createCollection(mCLName, mOpt);
		}catch(BaseException e){
			throw e;
		}
	}
	
	public void createSubCL(Sequoiadb sdb){
		try{
			CollectionSpace csDB = sdb.getCollectionSpace(csName);
			
			BSONObject sOpt = new BasicBSONObject();
			BSONObject sSubObj = new BasicBSONObject();
			sSubObj.put("a", 1);
			sOpt.put("ShardingKey", sSubObj);
			sOpt.put("ReplSize", 0);
			csDB.createCollection(sCLName, sOpt);
		}catch(BaseException e){
			throw e;
		}
	}
	
	public void attachCL(Sequoiadb sdb){
		try
		{
			CollectionSpace csDB = sdb.getCollectionSpace(csName);
			
			BSONObject opt = new BasicBSONObject();
			BSONObject lowBound = new BasicBSONObject();
			BSONObject upBound  = new BasicBSONObject();
			lowBound.put("a", 0);
			upBound.put("a", 200);
			opt.put("LowBound", lowBound);
			opt.put("UpBound", upBound);
			if(csDB.isCollectionExist(sCLName)){
				DBCollection clDB = csDB.getCollection( mCLName );
				clDB.attachCollection( csName + "." + sCLName, opt );
			}
		}catch(BaseException e){
			throw e;
		}
	}
	
}