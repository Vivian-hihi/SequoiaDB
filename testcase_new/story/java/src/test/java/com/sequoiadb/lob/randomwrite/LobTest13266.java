package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;

import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * FileName: LobTest13266.java test content:concurrent locking to the same range
 * of data segments to write lob testlink case:seqDB-13266
 * 
 * @author laojingtang
 * @Date 2017.12.1
 * @version 1.00
 * @update wuyan on 2019.5.20.
 */
public class LobTest13266 extends SdbTestBase {
	private String clName = "writelob13266";
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private static DBCollection dbcl = null;
	private static ObjectId oid = null;
	private byte[] testLobBuff = null;
	private int writeFailNum = 0;

	@BeforeClass
	public void setUp() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024," + "ReplSize:0,Compressed:true}";
		dbcl = RandomWriteLobUtil.createCL(cs, clName, clOptions);

		// put lob
		int writeSize = 1024 * 200;
		testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		oid = RandomWriteLobUtil.createAndWriteLob(dbcl, testLobBuff);
	}

	@Test
	public void testLob13266() {
		int offset = 10;
		int rewriteLobSize = 1024 * 100;
		byte[] writeBuff1 = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);
		byte[] writeBuff2 = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);

		LockAndRewriteLobTask lockAndRewriteLob1 = new LockAndRewriteLobTask(offset, writeBuff1);
		LockAndRewriteLobTask lockAndRewriteLob2 = new LockAndRewriteLobTask(offset, writeBuff2);

		lockAndRewriteLob1.start(5);
		lockAndRewriteLob2.start(5);

		Assert.assertTrue(lockAndRewriteLob1.isSuccess(), lockAndRewriteLob1.getErrorMsg());
		Assert.assertTrue(lockAndRewriteLob2.isSuccess(), lockAndRewriteLob2.getErrorMsg());

		// the concurrent operation has at least one locking fail,so the
		// writeFailNum > 0
		Assert.assertNotEquals(writeFailNum, 0, "at least one locking fail!" + writeFailNum);
		checkRewriteLobBuff(rewriteLobSize, offset, writeBuff1, writeBuff2);
	}

	@AfterClass
	public void tearDown() {
		try {
			if (cs.isCollectionExist(clName)) {
				cs.dropCollection(clName);
			}
		} finally {
			if (sdb != null) {
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
			try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection cl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
				try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
					lob.lockAndSeek(offset, rewriteLobBuff.length);
					lob.write(rewriteLobBuff);
				}
			} catch (BaseException e) {
				writeFailNum++;
				if (-320 != e.getErrorCode()) {
					throw e;
				}
			}
		}
	}

	private void checkRewriteLobBuff(int rewriteLobSize, int offset, byte[] rewriteBuff1, byte[] rewriteBuff2) {
		// check the rewrite lob
		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(dbcl, oid, rewriteLobSize, offset);
		// It is expected that the buffer may be rewriteBuff1 1 or rewriteBuff12
		byte[] expRewriteBuff = rewriteBuff1;
		if (!Arrays.equals(actBuff, expRewriteBuff)) {
			expRewriteBuff = rewriteBuff2;
		}
		RandomWriteLobUtil.assertByteArrayEqual(actBuff, expRewriteBuff);

		// check the all write lob
		byte[] expBuff = RandomWriteLobUtil.appendBuff(testLobBuff, expRewriteBuff, offset);
		try (DBLob lob = dbcl.openLob(oid)) {
			byte[] actAllLob = new byte[(int) lob.getSize()];
			lob.read(actAllLob);
			if (!Arrays.equals(actAllLob, expBuff)) {
				RandomWriteLobUtil.writeLobAndExpectData2File(lob, expBuff);
				Assert.fail("check actlob and expbuff different: oid=" + oid.toString());
			}
		}
	}

}
