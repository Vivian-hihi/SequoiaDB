package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10209
* @author xiaoni huang init
* @Date   2016.10.20
*/

public class Index10209 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10206";
	private String clName = "cl10206";
	
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//clear env
			CommLib.clearCL(sdb, csName, clName);
			CommLib.clearCS(sdb, csName);
			//create cs/cl
			sdb.createCollectionSpace(csName);
			BSONObject opt = new BasicBSONObject();
			opt.put("AutoIndexId", false);
			DBCollection clDB = sdb.getCollectionSpace(csName).
						createCollection(clName, opt);
			//create index
			BSONObject opt2 = new BasicBSONObject();
			opt2.put("SortBufferSize", 128);
			clDB.createIdIndex(opt2);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear env
			CommLib.clearCL(sdb, csName, clName);
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail("ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	@Parameters({"coordAddr"})
	public void testIndex10209(String coordAddr){
		Sequoiadb db  = null;
		DBCollection clDB = null;

		//-----drop index-----
		try{
			db = new Sequoiadb(coordAddr, "", "");
			clDB = db.getCollectionSpace(csName).getCollection(clName);
			
			clDB.dropIdIndex();
			//check results
			Index10209 Index10209 = new Index10209();
			Index10209.checkResult(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----alter cl-----

		try{
		    CollectionSpace csDB = db.getCollectionSpace(csName);
			BSONObject opt = new BasicBSONObject();
			opt.put("ReplSize", 1);
		    csDB.getCollection(clName).alterCollection(opt);
			
			//check results of catalog
			CommLib.checkCLOfCatalog(db, csName, clName);
			CommLib.checkCLOfDataRG(db, csName, clName);
			boolean rc = CommLib.compareDataAndCata(db, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create index-----
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put("SortBufferSize", 128);
			clDB.createIdIndex(opt);
			//check results
			Index10209 Index10209 = new Index10209();
			Index10209.checkResult(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
	}
	
	public void checkResult(Sequoiadb sdb){
		try{
			if(CommLib.isStandAlone(sdb)){
				DBCollection clDB = sdb.getCollectionSpace(csName).getCollection(clName);
				DBCursor idxInfo = clDB.getIndex("$id");
				if(idxInfo.hasNext()){
					clDB.dropIdIndex();
				}else{
					BSONObject opt = new BasicBSONObject();
					opt.put("SortBufferSize", 128);
					clDB.createIdIndex(opt);
				}
			}else{
				CommLib.checkIndex(sdb, csName, clName);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
}