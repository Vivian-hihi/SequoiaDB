package com.sequoiadb.backup;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * 此用例需要保证环境的每一台主机都有SdbTestBase.workDir指定的目录，并且权限应为777
 */
public class Backup7173 extends SdbTestBase {
	private Sequoiadb sdb;
	private String coordAddr;
	private String backupDir;
	private String groupName = "group7173";
	private String csName = "cs7173";
	private String clName = "cl7173";
	private int nodeNum = 1;
	private boolean success = false;

	@BeforeClass
	public void setUp() {
		this.coordAddr = SdbTestBase.coordUrl;
		this.backupDir = SdbTestBase.workDir;
		sdb = new Sequoiadb(coordAddr, "", "");
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("run mode is standalone,test case skip");
		}
		if (sdb.isRelicaGroupExist(groupName)) {
			if (sdb.isCollectionSpaceExist(csName)) {
				sdb.dropCollectionSpace(csName);
			}
			sdb.removeReplicaGroup(groupName);
		}
		BackupUtil.createRGAndNode(sdb, groupName, nodeNum);
		BSONObject options = new BasicBSONObject();
		options.put("Group", groupName);
		options.put("ReplSize", 0);
		DBCollection cl = sdb.createCollectionSpace(csName).createCollection(clName, options);
		BackupUtil.insertData(cl);
	}

	@AfterClass
	public void tearDown() {
		String path = backupDir + "/%g";
		BSONObject removeOption = new BasicBSONObject();
		removeOption.put("Path", path);
		try {
			if (success) {
				sdb.removeBackup(removeOption);
			}
		} catch (BaseException e) {
			if (e.getErrorCode() != -264)
				Assert.fail("clear env failed, errMsg:" + e.getMessage() + e.getStackTrace());
		} finally {
			sdb.dropCollectionSpace(csName);
			sdb.removeReplicaGroup(groupName);
			sdb.close();
		}
	}

	@Test
	public void test() {
		// set backup configure
		String backupName1 = "backup7173_1";
		String path = backupDir + "/%g";
		BSONObject option1 = new BasicBSONObject();
		option1.put("Name", backupName1);
		option1.put("GroupName", groupName);
		option1.put("Path", path);

		String backupName2 = "backup7173_2";
		BSONObject option2 = new BasicBSONObject();
		option2.put("Name", backupName2);
		option2.put("GroupName", groupName);
		option2.put("Path", path);
		// backup
		sdb.backup(option1);
		sdb.backup(option2);

		// list
		String hostName = sdb.getReplicaGroup(groupName).getMaster().getHostName();
		BSONObject listOption = new BasicBSONObject();
		listOption.put("Path", path);
		listOption.put("HostName", hostName);
		listOption.put("GroupName", groupName);

		BSONObject matcher = new BasicBSONObject();
		matcher.put("Name", backupName1);

		BSONObject selector = new BasicBSONObject();
		selector.put("Name", 1);
		selector.put("NodeName", 1);
		selector.put("GroupName", 1);
		selector.put("StartTime", 1);

		BSONObject orderBy = new BasicBSONObject();
		orderBy.put("GroupName", -1);

		DBCursor cursor = sdb.listBackup(listOption, matcher, selector, orderBy);
		while (cursor.hasNext()) {
			BSONObject record = cursor.getNext();
			if (record.containsField("Name")) {
				String actualBackupName = (String) record.get("Name");

				// check matcher
				Assert.assertEquals(actualBackupName, backupName1);

				// check selector
				int keySetSize = record.keySet().size();
				Assert.assertEquals(keySetSize, 4);
				Assert.assertTrue(record.containsField("Name"));
				Assert.assertTrue(record.containsField("NodeName"));
				Assert.assertTrue(record.containsField("GroupName"));
				Assert.assertTrue(record.containsField("StartTime"));
			}
		}
		cursor.close();

		// remove backup
		BSONObject removeOption = new BasicBSONObject();
		removeOption.put("Name", backupName1);
		removeOption.put("Path", path);

		try {
			sdb.removeBackup(removeOption);
			// check
			DBCursor cursor1 = sdb.listBackup(removeOption, null, null, null);
			while (cursor1.hasNext()) {
				BSONObject record = cursor1.getNext();
				if (record.containsField("Name")) {
					String actualBackupName = (String) record.get("Name");
					Assert.assertNotEquals(actualBackupName, backupName1);
					Assert.assertEquals(actualBackupName, backupName2);
				}
			}
		} catch (BaseException e) {
			if (e.getErrorCode() != -264)
				Assert.fail("remove backup failed, errMsg:" + e.getMessage());
		}
		success = true;
	}
}
