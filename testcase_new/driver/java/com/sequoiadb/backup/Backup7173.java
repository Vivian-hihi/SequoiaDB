package com.sequoiadb.backup;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * 此用例需要保证环境的每一台主机都有SdbTestBase.workDir指定的目录，并且权限应为777
 */
public class Backup7173 extends SdbTestBase {
    private Sequoiadb sdb;
    private SimpleDateFormat df = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS");
    private CommLib commlib = new CommLib();
    private String coordAddr;
    private String backupDir;

    @BeforeClass
    public void setUp() {
        this.coordAddr = SdbTestBase.coordUrl;
        this.backupDir = SdbTestBase.workDir;
        try {
            System.out.println("the TestCase: " + this.getClass().getName() +
                    " begin at:" + df.format(new Date().getTime()));
            sdb = new Sequoiadb(coordAddr, "", "");
            if (commlib.isStandAlone(sdb)) {
                throw new SkipException("run mode is standalone,test case skip");
            }
        } catch (BaseException e) {
            Assert.fail("prepare env failed" + e.getMessage());
        }
    }

    @AfterClass
    public void tearDown() {
        String path = backupDir + "/%g";
        BSONObject removeOption = new BasicBSONObject();
        removeOption.put("Path", path);
        try {
            sdb.removeBackup(removeOption);
            System.out.println("the TestCase: " + this.getClass().getName() +
                    " end at:" + df.format(new Date().getTime()));
            sdb.disconnect();
        } catch (BaseException e) {
            if (e.getErrorCode() != -264)
                Assert.fail("clear env failed, errMsg:" + e.getMessage());
        }
    }

    @Test
    public void test() {
        //set backup configure
        ArrayList<String> dataGroups = commlib.getDataGroupNames(sdb);
        System.out.println("dataGroups:" + dataGroups);
        String backupName1 = "backup7173_1";
        String path = backupDir + "/%g";
        BSONObject option1 = new BasicBSONObject();
        option1.put("Name", backupName1);
        option1.put("GroupName", dataGroups);
        option1.put("Path", path);

        String backupName2 = "backup7173_2";
        BSONObject option2 = new BasicBSONObject();
        option2.put("Name", backupName2);
        option2.put("GroupName", dataGroups);
        option2.put("Path", path);
        System.out.println(option1.toString());
        //backup
        try {
            sdb.backupOffline(option1);
            sdb.backupOffline(option2);
        } catch (BaseException e) {
            Assert.fail("backup failed, errMsg:" + e.getMessage());
        }

        //list
        String groupName = dataGroups.get(0);
        String hostName = sdb.getReplicaGroup(groupName).getMaster().getHostName();
        BSONObject listOption = new BasicBSONObject();
        listOption.put("Path", path);
        System.out.println("Path:" + path);
        listOption.put("HostName", hostName);
        System.out.println("HostName:" + hostName);
        listOption.put("GroupName", groupName);
        System.out.println("groupName:" + groupName);

        BSONObject matcher = new BasicBSONObject();
        matcher.put("Name", backupName1);
        System.out.println("Name:" + backupName1);

        BSONObject selector = new BasicBSONObject();
        selector.put("Name", 1);
        selector.put("NodeName", 1);
        selector.put("GroupName", 1);
        selector.put("StartTime", 1);

        BSONObject orderBy = new BasicBSONObject();
        orderBy.put("GroupName", -1);

        try {
            DBCursor cursor = sdb.listBackup(listOption, matcher, selector, orderBy);
            while (cursor.hasNext()) {
                BSONObject record = cursor.getNext();
                System.out.println("groupName:" + groupName);
                if (record.containsField("Name")) {
                    String actualBackupName = (String) record.get("Name");

                    //check matcher
                    Assert.assertEquals(actualBackupName, backupName1);

                    //check selector
                    int keySetSize = record.keySet().size();
                    Assert.assertEquals(keySetSize, 4);
                    Assert.assertTrue(record.containsField("Name"));
                    Assert.assertTrue(record.containsField("NodeName"));
                    Assert.assertTrue(record.containsField("GroupName"));
                    Assert.assertTrue(record.containsField("StartTime"));
                }
            }
            cursor.close();
        } catch (BaseException e) {
            Assert.fail("list backup failed, errMsg:" + e.getMessage());
        }


        //remove backup
        BSONObject removeOption = new BasicBSONObject();
        removeOption.put("Name", backupName1);
        removeOption.put("Path", path);

        System.out.println(removeOption.toString());

        try {
            sdb.removeBackup(removeOption);
            //check
            DBCursor cursor = sdb.listBackup(removeOption, null, null, null);
            while (cursor.hasNext()) {
                BSONObject record = cursor.getNext();
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
    }
}
