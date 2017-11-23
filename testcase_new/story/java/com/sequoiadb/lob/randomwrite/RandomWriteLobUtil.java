package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;

import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
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

    private static ArrayList<String> groupList;

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


    /**
     * get the buff MD5 value
     *
     * @param inbuff the object of need to get the MD5
     * @return the MD5 value
     */
    static String getMd5(byte[] inbuff) {
        MessageDigest md5;
        String value = "";
        try {
            md5 = MessageDigest.getInstance("MD5");

            md5.update(inbuff);
            BigInteger bi = new BigInteger(1, md5.digest());
            value = bi.toString(16);
            //else{
            //Assert.fail("invalid parameter!");
            //}
        } catch (NoSuchAlgorithmException e) {
            log.severe(e.getMessage());
        }
        return value;
    }

    static byte[] appendBuff(byte[] testLobBuff, byte[] rewriteBuff, int offset) {
        byte[] appendBuff;
        if (testLobBuff.length >= offset + rewriteBuff.length) {
            appendBuff = new byte[testLobBuff.length];
        } else {
            appendBuff = new byte[offset + rewriteBuff.length];
        }
        System.arraycopy(testLobBuff, 0, appendBuff, 0, testLobBuff.length);
        System.arraycopy(rewriteBuff, 0, appendBuff, offset, rewriteBuff.length);
        return appendBuff;
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
        groupList = sdb.getReplicaGroupNames();
        groupList.remove("SYSCatalogGroup");
        groupList.remove("SYSCoord");
        groupList.remove("SYSSpare");
        return groupList;
    }

    static String chooseDataGroups(Sequoiadb sdb, int groupsNum) {
        groupList = getDataGroups(sdb);
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
        groupList = getDataGroups(sdb);
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
            Assert.fail("\nexpect: " + Arrays.toString(expect)
                    + "\nbut actual: " + Arrays.toString(actual) + "\n" + msg + "\n");
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
        return readLob(dbcl.openLob(id));
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
}
