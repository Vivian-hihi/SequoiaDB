package com.sequoiadb.rename;

import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCS_16094.java cs数据在多个组上，并发修改cs名和读写删查LOB
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16094 extends SdbTestBase{
	
	private String csName = "renameCS_16094";
	private String clName = "rename_CL_16094";
	private String newCLName = "rename_CL_16094_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private List<ObjectId> lobIdList = null;
	private int fileSize = 1024 * 1024;
	private String MD5 = null;
	private int lobNum = 10;
	private byte[] data = null;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.createCollectionSpace(csName);
		BSONObject options = new BasicBSONObject();
		cl = cs.createCollection(clName, options);
		
		data = new byte[fileSize];
		Random random = new Random();
		random.nextBytes(data);
		MD5 = RenameUtil.getMd5(data);
		lobIdList = RenameUtil.putLob(cl, data, lobNum);
		
	}
	
	@Test
	public void test(){ 
		RenameCLThread renameThread = new RenameCLThread();
		PutLobThread putThread = new PutLobThread();
		ReadLobThread readThread = new ReadLobThread();
		DeleteLobThread deleteThread = new DeleteLobThread();
		ListLobThread listThread = new ListLobThread();
		
		renameThread.start();
		putThread.start();
		readThread.start();
		deleteThread.start();
		listThread.start();
		
		Assert.assertTrue(renameThread.isSuccess(), renameThread.getErrorMsg());
		System.out.println(putThread.isSuccess());
		System.out.println(readThread.isSuccess());
		System.out.println(deleteThread.isSuccess());
		System.out.println(listThread.isSuccess());
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			checkLob(db, csName, newCLName);
		}finally{
			db.close();
		}
	}
	
	@AfterClass
	public void tearDown(){
		CommLib.clearCS(sdb, csName);
		if(sdb!=null){
			sdb.close();
		}
	}
	
	private class RenameCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Thread.sleep(3000);
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clName, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
	private class PutLobThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				RenameUtil.putLob(sdbcl, data, lobNum/2);
			}finally {
				db.close();
			}
		}
	}
	
	private class ReadLobThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				sdbcl.listLobs();
				byte[] data = new byte[fileSize];
				for(int i = lobNum/2; i < lobNum; i++){
					DBLob lob = sdbcl.openLob(lobIdList.get(0));
					lob.read(data);
					lob.close();
				}
			}finally {
				db.close();
			}
		}
	}
	
	private class DeleteLobThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				for (int i = 0; i < lobNum/2; i++) {
					sdbcl.removeLob(lobIdList.get(i));
				}
			}finally {
				db.close();
			}
		}
	}
	
	private class ListLobThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				for (int i = 0; i < 10; i++) {
					Thread.sleep(500);
					sdbcl.listLobs();
				}
			}finally {
				db.close();
			}
		}
	}
	
	private void checkLob(Sequoiadb db, String csName, String clNmae){
		DBCollection cl = db.getCollectionSpace(csName).getCollection(clNmae);
		DBCursor cur = cl.listLobs();
		while(cur.hasNext()){
			BSONObject idObj = cur.getNext();
			ObjectId id = (ObjectId) idObj.get("Oid");
			DBLob lob = cl.openLob(id);
			byte[] data = new byte[fileSize];
			lob.read(data);
			String actMD5 = RenameUtil.getMd5(data);
			if(!actMD5.equals(MD5)){
				Assert.fail("file md5 error, exp: " + MD5 + ", act: " + actMD5);
			}
		}
	}
	
}
