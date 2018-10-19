package com.sequoiadb.rename;

import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.SkipException;
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
 * @Description RenameCS_16129.java cs数据在多个组上，并发修改cs名和读写删查LOB
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16129 extends SdbTestBase{
	
	private String csName = "renameCS_16129";
	private String newCSName = "renameCS_16129_new";
	private String clName = "renameCS_CL_16129";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private String groupName1 = null;
	private String groupName2 = null;
	private List<ObjectId> lobIdList = null;
	private int fileSize = 1024 * 1024;
	private String MD5 = null;
	private int lobNum = 10;
	private byte[] data = null;
	
	@BeforeClass(alwaysRun=true)
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if(CommLib.isStandAlone(sdb)){
			throw new SkipException("skip StandAlone");
		}
		List<String> rgNames = CommLib.getDataGroupNames(sdb);
		if(rgNames.size() <= 1){
			throw new SkipException("current environment less than tow groups");
		}
		groupName1 = rgNames.get(0);
		groupName2 = rgNames.get(1);
		cs = sdb.createCollectionSpace(csName);
		BSONObject options = new BasicBSONObject();
		options.put("Group", groupName1);
		options.put("ShardingType", "hash");
		options.put("ShardingKey", new BasicBSONObject("a", 1));
		cl = cs.createCollection(clName, options);
		cl.split(groupName1, groupName2, 50);
		
		data = new byte[fileSize];
		Random random = new Random();
		random.nextBytes(data);
		MD5 = RenameUtil.getMd5(data);
		lobIdList = RenameUtil.putLob(cl, data, lobNum);
		
	}
	
	@Test
	public void test(){ 
		RenameCSThread renameThread = new RenameCSThread();
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
			RenameUtil.checkRenameCSResult(db, csName, newCSName, 1);
			checkLob(db, newCSName, clName);
		}finally{
			db.close();
		}
	}
	
	@AfterClass(alwaysRun=false)
	public void tearDown(){
		CommLib.clearCS(sdb, newCSName);
		if(sdb!=null){
			sdb.close();
		}
	}
	
	private class RenameCSThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Thread.sleep(3000);
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				db.renameCollectionSpace(csName, newCSName);
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
		DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
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
