package com.sequoias3.testcommon;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;

public class TestResidualData extends S3TestBase {
    Sequoiadb db = null;
    int errorCount = 0;
    String printInfo = "";
    String printDirInfo = "";
    String bucketId = "";
    String enabledBucketId = "";

    @BeforeClass
    private void setUp() {
    }

    @Test
    private void printResidualData() throws Exception {
        List<String> csNames = new ArrayList<String>();
        List<String> s3CSNames = new ArrayList<String>();
        List<String> s3DataCSNames = new ArrayList<String>();
        db = new Sequoiadb(S3TestBase.coordUrl, "", "");
        csNames = db.getCollectionSpaceNames();

        for (String csName : csNames) {
            if (csName.startsWith("S3_")) {
                if (csName.contains("Meta")) {
                    s3CSNames.add(csName);
                } else {
                    s3DataCSNames.add(csName);
                }
            }
        }

        for (String csName : s3CSNames) {
            CollectionSpace cs = db.getCollectionSpace(csName);
            List<DBCollection> clList = new ArrayList<DBCollection>();
            List<String> clNameList = cs.getCollectionNames();
            for (String csclName : clNameList) {
                String clname = csclName.substring(cs.getName().length() + 1);
                // S3_SYS_Meta.S3_IDGenerator为ID生成表，属于s3内部表，不需要打印和校验，S3_SYS_Meta.S3_ObjectDir目录表单独校验打印
                if (!clname.equals("S3_IDGenerator") && !clname.equals("S3_ObjectDir")) {
                    clList.add(cs.getCollection(clname));
                }
            }
            printResidualMetaData(cs, clList);
            printObjectDirData(cs);
        }

        for (String csName : s3DataCSNames) {
            CollectionSpace cs = db.getCollectionSpace(csName);
            List<DBCollection> clList = new ArrayList<DBCollection>();
            List<String> clNameList = cs.getCollectionNames();
            for (String csclName : clNameList) {
                String clname = csclName.substring(cs.getName().length() + 1);
                clList.add(cs.getCollection(clname));
            }
            printResidualData(cs, clList);
        }

        writeToFile();
    }

    private void printResidualMetaData(CollectionSpace cs, List<DBCollection> clList) {
        DBCursor cursor = null;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy");
        Date date = new Date();
        String s3SysDataRegionSpaceName = "S3_SYS_Data_" + sdf.format(date);
        try {
            for (DBCollection cl : clList) {
                printInfo += "\n===============begin print " + cs.getName() + "." + cl.getName()
                        + " data============\n";
                cursor = cl.query();
                if (cursor.hasNext()) {
                    while (cursor.hasNext()) {
                        if (cursor.getNext().containsField("Name")) {
                            if (!cursor.getCurrent().get("Name").equals(S3TestBase.s3UserName)
                                    && !cursor.getCurrent().get("Name").equals(S3TestBase.bucketName)
                                    && !cursor.getCurrent().get("Name").equals(S3TestBase.enableVerBucketName)
                                    && !cursor.getCurrent().get("Name").equals(s3SysDataRegionSpaceName)) {
                                printInfo += cursor.getCurrent().toString() + "\n";
                                errorCount++;
                            } else if (cursor.getCurrent().get("Name").equals(S3TestBase.bucketName)) {
                                bucketId = cursor.getCurrent().get("ID").toString();
                            } else if (cursor.getCurrent().get("Name").equals(S3TestBase.enableVerBucketName)) {
                                enabledBucketId = cursor.getCurrent().get("ID").toString();
                            }
                        } else {
                            printInfo += cursor.getCurrent().toString() + "\n";
                            errorCount++;
                        }
                    }
                }
                printInfo += "===============end print " + cs.getName() + "." + cl.getName() + " data==============\n";
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void printResidualData(CollectionSpace cs, List<DBCollection> clList) {
        DBCursor cursor = null;
        try {
            for (DBCollection cl : clList) {
                printInfo += "\n===============begin print " + cs.getName() + "." + cl.getName()
                        + " data============\n";
                cursor = cl.listLobs();
                if (cursor.hasNext()) {
                    while (cursor.hasNext()) {
                        printInfo += cursor.getNext().toString() + "\n";
                        errorCount++;
                    }
                }
                printInfo += "===============end print " + cs.getName() + "." + cl.getName() + " data==============\n";
            }
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), SDBError.SDB_DMS_NOTEXIST.getErrorCode(),
                    "getCollection ObjectDataList failed");
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void printObjectDirData(CollectionSpace cs) {
        DBCollection cl = cs.getCollection("S3_ObjectDir");
        DBCursor cursor = null;
        try {
            printDirInfo += "\n===============begin print " + cs.getName() + "." + cl.getName() + " data============\n";
            cursor = cl.query();
            if (cursor.hasNext()) {
                while (cursor.hasNext()) {
                    if (!cursor.getNext().get("BucketId").equals(bucketId)
                            && !cursor.getCurrent().get("BucketId").equals(enabledBucketId)) {
                        printDirInfo += cursor.getCurrent().toString() + "\n";
                        errorCount++;
                    }
                }
            }
            printDirInfo += "===============end print " + cs.getName() + "." + cl.getName() + " data==============\n";
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void writeToFile() throws Exception {
        File file = new File(S3TestBase.installPath + "/tools/sequoias3/log/residualdata.log");
        if (!file.exists()) {
            file.createNewFile();
        }

        FileWriter fw = new FileWriter(file.getAbsoluteFile(), false);
        BufferedWriter bw = new BufferedWriter(fw);
        bw.write(printInfo);
        bw.close();

        // 打印日志表
        File file2 = new File(S3TestBase.installPath + "/tools/sequoias3/log/residualDirData.log");
        if (!file2.exists()) {
            file2.createNewFile();
        }

        FileWriter fw2 = new FileWriter(file2.getAbsoluteFile(), false);
        BufferedWriter bw2 = new BufferedWriter(fw2);
        bw2.write(printDirInfo);
        bw2.close();

        if (errorCount != 0) {
            throw new Exception("There is data residue problem");
        }

    }

    @AfterClass
    private void tearDown() throws Exception {
        if (db != null) {
            db.close();
        }
    }
}
