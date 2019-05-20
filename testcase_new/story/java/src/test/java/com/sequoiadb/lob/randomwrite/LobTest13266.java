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

	@BeforeClass
	public void setupClass() {
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
		int rewriteLobSize = 1024 * 4;
		byte[] writeBuff1 = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);
		byte[] writeBuff2 = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);

		LockAndRewriteLobTask lockAndRewriteLob1 = new LockAndRewriteLobTask(offset, writeBuff1);
		LockAndRewriteLobTask lockAndRewriteLob2 = new LockAndRewriteLobTask(offset, writeBuff2);

		lockAndRewriteLob1.start();
		lockAndRewriteLob2.start();

		if (lockAndRewriteLob1.isSuccess()) {
			Assert.assertFalse(lockAndRewriteLob2.isSuccess(), lockAndRewriteLob2.getErrorMsg());
			checkRewriteLobBuff(rewriteLobSize, offset, writeBuff1);
		} else {
			Assert.assertTrue(lockAndRewriteLob2.isSuccess(), lockAndRewriteLob2.getErrorMsg());
			checkRewriteLobBuff(rewriteLobSize, offset, writeBuff2);
		}
	}

	@AfterClass
	public void afterClass() {
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
			DBLob lob = null;
			try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection cl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
				lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);
				lob.lockAndSeek(offset, rewriteLobBuff.length);
				lob.write(rewriteLobBuff);
				lob.close();
			}
		}
	}

	private void checkRewriteLobBuff(int rewriteLobSize, int offset, byte[] rewriteBuff) {
		// check the rewrite lob
		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(dbcl, oid, rewriteLobSize, offset);
		RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);

		// check the all write lob
		byte[] expBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff, offset);
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
