package com.sequoiadb.lob.randomwrite;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/*
 * @FileName seqDB-13252: 加锁写入过程中删除lob 
 * @Author wuyan modify
 * @Date 2019-06-11
 * @Version 1.00
 */
public class TestLob13252 extends SdbTestBase {
	private String clName = "writelob13252";
	private int lobSize = 1024 * 1024 * 5;
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private ObjectId lobOid = null;
	private byte[] lobData = null;

	@BeforeClass
	public void setUp() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		// create cs cl
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
		cl = cs.createCollection(clName, clOpt);

		// write lob
		lobData = RandomWriteLobUtil.getRandomBytes(lobSize);
		lobOid = RandomWriteLobUtil.createAndWriteLob(cl, lobData);
	}

	@Test()
	public void testLob13252() {
		int offset = 1024 * 1024 * 1;
		int rewriteLobSize = 1024 * 1024 * 3;
		byte[] lockAndRewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);
		LockAndRewriteLobTask lockAndRewriteLob = new LockAndRewriteLobTask(offset, lockAndRewriteBuff);
		RemoveLobTask removeLob = new RemoveLobTask();
		lockAndRewriteLob.start();
		removeLob.start();

		if (lockAndRewriteLob.isSuccess()) {
			if (!removeLob.isSuccess()) {
				Assert.assertTrue(!removeLob.isSuccess(), removeLob.getErrorMsg());
				BaseException e = (BaseException) (removeLob.getExceptions().get(0));
				if (-320 != e.getErrorCode() && -317 != e.getErrorCode()) {
					Assert.fail("removeLob must fail:" + e.getErrorCode() + " " + removeLob.getErrorMsg());
				}				
				RandomWriteLobUtil.checkRewriteLobResult(cl, lobOid, offset, lockAndRewriteBuff, lobData);
			} else {
				// can't determine the status of the server, and maybe all
				// operations are sucessfull,
				Assert.assertTrue(removeLob.isSuccess());
				// check the remove result
				DBCursor listCursor1 = cl.listLobs();
				Assert.assertEquals(listCursor1.hasNext(), false, "list lob not null");
				listCursor1.close();
			}
		} else if (!lockAndRewriteLob.isSuccess()) {
			Assert.assertTrue(removeLob.isSuccess(), removeLob.getErrorMsg());
			BaseException e = (BaseException) (lockAndRewriteLob.getExceptions().get(0));
			if (-268 != e.getErrorCode() && -4 != e.getErrorCode()) {
				Assert.fail("lockAndRewriteLob must fail:" + e.getErrorCode() + " " + removeLob.getErrorMsg());
			}
			// check the remove result
			DBCursor listCursor1 = cl.listLobs();
			Assert.assertEquals(listCursor1.hasNext(), false, "list lob not null");
			listCursor1.close();
		} else {
			Assert.fail("unexpected result! lockAndRewriteLob:" + lockAndRewriteLob.isSuccess() + " removeLob: "
					+ removeLob.isSuccess());
		}

	}

	@AfterClass
	public void tearDown() {
		try {
            if (cs.isCollectionExist(clName)) {
            	cs.dropCollection(clName);
            }
        } finally {
            if (null != sdb) {
                sdb.close();
            }
        }		
	}

	private class LockAndRewriteLobTask extends SdbThreadBase {
		private int offset;
		private byte[] rewriteLobBuff;

		public LockAndRewriteLobTask(int offset, byte[] rewriteLobBuff) {
			this.offset = offset;
			this.rewriteLobBuff = rewriteLobBuff;
		}

		@Override
		public void exec() throws Exception {
			DBLob lob = null;
			try (Sequoiadb sdb1 = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection cl1 = sdb1.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
				lob = cl1.openLob(lobOid, DBLob.SDB_LOB_WRITE);
				lob.lockAndSeek(offset, rewriteLobBuff.length);
				lob.write(rewriteLobBuff);
				lob.close();
			}
		}
	}

	private class RemoveLobTask extends SdbThreadBase {
		@Override
		public void exec() throws Exception {
			try (Sequoiadb sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");) {
				DBCollection cl2 = sdb2.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
				cl2.removeLob(lobOid);
			}
		}
	}

}
