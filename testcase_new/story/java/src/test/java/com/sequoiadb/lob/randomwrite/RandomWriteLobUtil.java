package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.DataProvider;

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.logging.Logger;

/**
 * FileName: RandomWriteLobUtil.java
 * public call function for test basicOperation
 *
 * @author laojingtang
 * @version 1.00
 */
class RandomWriteLobUtil {

    final private static Logger log = Logger.getLogger(RandomWriteLobUtil.class.getName());

    static DBCollection createCL(CollectionSpace cs, String clName, String option) {
        DBCollection cl = null;
        BSONObject options = (BSONObject) JSON.parse(option);
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }

            cl = cs.createCollection(clName, options);
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        return cl;
    }

    static DBCollection createCL(CollectionSpace cs, String clName) {
        DBCollection cl = null;
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
            cl = cs.createCollection(clName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        return cl;
    }

    static byte[] appendBuff(byte[] oldBuff, byte[] buff4Append, int offset) {
        byte[] newBuff;
        if (oldBuff.length >= offset + buff4Append.length) {
            newBuff = new byte[oldBuff.length];
        } else {
            newBuff = new byte[offset + buff4Append.length];
        }
        System.arraycopy(oldBuff, 0, newBuff, 0, oldBuff.length);
        System.arraycopy(buff4Append, 0, newBuff, offset, buff4Append.length);
        return newBuff;
    }

    /**
     * generating byte to write lob
     *
     * @param length generating byte stream size
     * @return byte[] bytes
     */
    static byte[] getRandomBytes(int length) {
        byte[] bytes = new byte[length];
        Random random = new Random();
        random.nextBytes(bytes);
        return bytes;
    }

    static String getRandomString(int size) {
        return Arrays.toString(getRandomBytes(size));
    }

    static ArrayList<String> getDataGroups(Sequoiadb sdb) {
        ArrayList<String> groupList = sdb.getReplicaGroupNames();
        groupList.remove("SYSCatalogGroup");
        groupList.remove("SYSCoord");
        groupList.remove("SYSSpare");
        return groupList;
    }

    static String chooseDataGroups(Sequoiadb sdb, int groupsNum) {
        ArrayList<String> groupList = getDataGroups(sdb);
        int length = (groupsNum > groupList.size()) ? groupList.size() : groupsNum;
        String ret = "";
        for (int i = 0; i < length; i++) {
            ret = ret + "'" + groupList.get(i) + "',";
        }
        return (ret.length() == 0) ? ret : ret.substring(0, ret.length() - 1);
    }

    static String getSrcGroupName(Sequoiadb sdb, String csName, String clName) {
        String groupName = "";
        String cond = String.format("{Name:\"%s.%s\"}", csName, clName);
        DBCursor cr = sdb.getSnapshot(8, cond, null, null);
        while (cr.hasNext()) {
            BSONObject obj = cr.getNext();

            BasicBSONObject doc = (BasicBSONObject) obj;
            doc.getString("Name");
            BasicBSONList subdoc = (BasicBSONList) doc.get("CataInfo");
            BasicBSONObject elem = (BasicBSONObject) subdoc.get(0);
            groupName = elem.getString("GroupName");
        }
        return groupName;
    }

    static String getSplitGroupName(Sequoiadb sdb, String groupName) {
        String tarRgName = "";
        List<String> groupList = getDataGroups(sdb);
        for (String name : groupList) {
            if (!name.equals(groupName)) {
                tarRgName = name;
                break;
            }
        }
        return tarRgName;
    }

    static void assertByteArrayEqual(byte[] actual, byte[] expect) {
        assertByteArrayEqual(actual, expect, "");
    }

    static void assertByteArrayEqual(byte[] actual, byte[] expect, String msg) {
        if (!Arrays.equals(actual, expect)) {
            String workDirPath = SdbTestBase.getWorkDir();
            File workDir = new File(workDirPath);
            if (!workDir.isDirectory())
                throw new RuntimeException("the path can not use: " + workDirPath);

            String callerClassName = getCallerName();
            File fileActual = new File(workDirPath + File.separator + callerClassName + "_actual");
            File fileExpect = new File(workDirPath + File.separator + callerClassName + "_expect");
            try {
                if (fileActual.exists()) {
                    fileActual.delete();
                    fileActual.createNewFile();
                }
                if (fileExpect.exists()) {
                    fileExpect.delete();
                    fileExpect.createNewFile();
                }

                try (FileOutputStream out = new FileOutputStream(fileActual)) {
                    out.write(actual);
                    out.flush();
                }
                try (FileOutputStream out = new FileOutputStream(fileExpect)) {
                    out.write(expect);
                    out.flush();
                }

                Assert.fail(msg + "; data is written into files in " + workDirPath);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static String getCallerName() {
        StackTraceElement[] stackTrace = Thread.currentThread().getStackTrace();
        String thisClassName = stackTrace[1].getClassName();
        String currClassName = null;
        for (int i = 2; i < stackTrace.length; ++i) {
            currClassName = stackTrace[i].getClassName();
            if (!currClassName.equals(thisClassName)) {
                break;
            }
        }
        String classFullName = currClassName;
        String[] classNameArr = classFullName.split("\\.");
        String simpleClassName = classNameArr[classNameArr.length - 1];
        return simpleClassName;
    }

    /**
     * 把lob和期望值分别写到以该lob oid开头的文件，路径由SdbTestBase.getWorkDir()指定
     *
     * @param lob
     * @param expect
     */
    static void writeLobAndExpectData2File(DBLob lob, byte[] expect) {
        String path = SdbTestBase.getWorkDir();

        File dir = new File(path);
        if (!dir.isDirectory())
            throw new RuntimeException("the path can not use: " + path);
        path = dir.toPath().toString();
        String lobID = lob.getID().toString();
        File fileActual = new File(path + File.separator + lobID + "_actual");
        File fileExpect = new File(path + File.separator + lobID + "_expect");

        if (fileActual.exists()) {
            fileActual.delete();
            try {
                fileActual.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        if (fileExpect.exists()) {
            fileExpect.delete();
            try {
                fileExpect.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        try (FileOutputStream out = new FileOutputStream(fileActual)) {
            lob.read(out);
            out.flush();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        try (FileOutputStream out = new FileOutputStream(fileExpect)) {
            out.write(expect);
            out.flush();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * @param dbLob
     * @return return all the lob data.
     */
    static byte[] readLob(DBLob dbLob, int retryTime) {
        for (int i = 0; i < retryTime; i++) {
            try {
                ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                dbLob.read(outputStream);
                return outputStream.toByteArray();
            } catch (BaseException e) {
                log.warning(e.getMessage());
                if (e.getErrorCode() != SDBError.SDB_FNE.getErrorCode())
                    throw e;
                else if (i == 5) {
                    throw e;
                } else {
                    try {
                        Thread.sleep(500);
                    } catch (InterruptedException e1) {
                    }
                }
            }
        }
        //should never come here
        throw new RuntimeException("read lob fail after " + retryTime + " times");
    }


    static byte[] readLob(DBCollection dbcl, ObjectId id, int retryTime) {
        DBLob lob = dbcl.openLob(id);
        byte[] b = readLob(lob, retryTime);
        lob.close();
        return b;
    }

    static byte[] readLob(DBLob lob) {
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        lob.read(outputStream);
        return outputStream.toByteArray();
    }

    static byte[] readLob(DBCollection dbcl, ObjectId id) {
        byte[] res;
        try (DBLob lob = dbcl.openLob(id)) {
            res = readLob(lob);
        }
        return res;
    }

    static byte[] readLob(Sequoiadb db, String csName, String clName, ObjectId id) {
        return readLob(db.getCollectionSpace(csName).getCollection(clName),
                id);
    }

    /**
     * convenient method fro creating empty lob.
     *
     * @param dbcl
     * @return
     */
    static ObjectId createEmptyLob(DBCollection dbcl, ObjectId id) {
        try (DBLob lob = dbcl.createLob(id)) {
            return lob.getID();
        }
    }

    static ObjectId createEmptyLob(DBCollection dbcl) {
        return createEmptyLob(dbcl, null);
    }

    /**
     * convenient method for creating lob and write some data to this lob.
     *
     * @param dbcl
     * @param id
     * @return
     */
    static ObjectId createAndWriteLob(DBCollection dbcl, ObjectId id, byte[] data) {
        DBLob lob = dbcl.createLob(id);
        lob.write(data);
        lob.close();
        return lob.getID();
    }

    static ObjectId createAndWriteLob(DBCollection dbcl, byte[] data) {
        return createAndWriteLob(dbcl, null, data);
    }

    static ObjectId createAndWriteLob(Sequoiadb db, String csName, String clName, ObjectId id, byte[] data) {
        return createAndWriteLob(db.getCollectionSpace(csName).getCollection(clName),
                id, data);
    }

    static ObjectId createAndWriteLob(Sequoiadb db, String csName, String clName, byte[] data) {
        return createAndWriteLob(db.getCollectionSpace(csName).getCollection(clName), data);
    }

    static byte[] seekAndReadLob(DBCollection dbcl, ObjectId lobid, int readSize, int offset) {
        byte[] b = new byte[readSize];
        try (DBLob lob = dbcl.openLob(lobid)) {
            lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
            lob.read(b);
        }
        return b;
    }


    public static class LobSizedataProvider {
        @DataProvider(name = "lobSizeDataProvider")
        public static Object[][] lobSizeDataProvider() {
            return new Object[][]{
                    {1024*1024},
                    {1024*1024*10},
                    {1024*1024*100}
            };
        }
    }
}
