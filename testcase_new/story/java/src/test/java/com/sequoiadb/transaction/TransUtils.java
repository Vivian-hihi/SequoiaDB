package com.sequoiadb.transaction;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Domain;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * 
 * @description Utils for this package class
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TransUtils {

    public static final int FLG_INSERT_CONTONDUP = 0x00000001;
    public static final int TIMEOUT = SdbTestBase.timeOutLen - 20;

    public static CollectionSpace createCS(String csName, Sequoiadb db) throws BaseException {
        CollectionSpace tmp = null;
        try {
            if (db.isCollectionSpaceExist(csName)) {
                db.dropCollectionSpace(csName);
            }
            tmp = db.createCollectionSpace(csName);

        } catch (BaseException e) {
            throw e;
        }
        return tmp;
    }

    public static CollectionSpace createCS(String csName, Sequoiadb db, String option) throws BaseException {
        CollectionSpace tmp = null;
        try {
            if (db.isCollectionSpaceExist(csName)) {
                db.dropCollectionSpace(csName);
            }
            tmp = db.createCollectionSpace(csName, (BSONObject) JSON.parse(option));

        } catch (BaseException e) {
            throw e;
        }
        return tmp;
    }

    public static Domain createDomain(Sequoiadb sdb, String name, ArrayList<String> groupArr, int size,
            boolean autoSplit) throws BaseException {
        Domain domain = null;
        try {
            if (sdb.isDomainExist(name)) {
                domain = sdb.getDomain(name);
            } else {
                StringBuilder groups = new StringBuilder();
                String option = new String();
                for (int i = 0; i < groupArr.size() && i < size; i++) {
                    groups.append("\"").append(groupArr.get(i)).append("\",");
                }
                groups.deleteCharAt(groups.length() - 1);
                groups.insert(0, "[");
                groups.append("]");
                if (autoSplit) {
                    option = "{\"Groups\":" + groups + ",\"AutoSplit\":true}";
                } else {
                    option = "{\"Groups\":" + groups + ",\"AutoSplit\":false}";
                }
                domain = sdb.createDomain(name, (BSONObject) JSON.parse(option));
            }

        } catch (BaseException e) {
            throw e;
        }
        return domain;
    }

    public static DBCollection createCL(String clName, CollectionSpace cs, String option) throws BaseException {
        DBCollection tmp = null;
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
            tmp = cs.createCollection(clName, (BSONObject) JSON.parse(option));
        } catch (BaseException e) {
            throw e;
        }
        return tmp;
    }

    // 检查某集合是否仅含一个dest记录
    public static boolean isCollectionContainThisJSON(DBCollection cl, String dest) throws BaseException {
        BSONObject bobj = (BSONObject) JSON.parse(dest);
        ArrayList<Object> resaults = new ArrayList<Object>();
        DBCursor dc = null;
        try {
            dc = cl.query(bobj, null, null, null);
            while (dc.hasNext()) {
                resaults.add(dc.getNext());
            }
            if (resaults.size() != 1) {
                return false;
            }
            BSONObject actual = (BSONObject) resaults.get(0);
            actual.removeField("_id");
            bobj.removeField("_id");
            if (bobj.equals(actual)) {
                return true;
            } else {
                return false;
            }
        } catch (BaseException e) {
            throw e;
        }
    }

    public static String getKeyStack(Exception e, Object classObj) {
        StringBuffer stackBuffer = new StringBuffer();
        StackTraceElement[] stackElements = e.getStackTrace();
        for (int i = 0; i < stackElements.length; i++) {
            if (stackElements[i].toString().contains(classObj.getClass().getName())) {
                stackBuffer.append(stackElements[i].toString()).append("\r\n");
            }
        }
        String str = stackBuffer.toString();
        return str.substring(0, str.length() - 2);
    }

    public static ArrayList<String> getGroupName(Sequoiadb sdb, String csName, String clName) throws BaseException {
        DBCursor dbc = null;
        ArrayList<String> resault = new ArrayList<String>();
        try {
            ArrayList<String> groups = CommLib.getDataGroupNames(sdb);
            dbc = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{Name:\"" + csName + "." + clName + "\"}", null, null);
            BasicBSONList list = null;
            if (dbc.hasNext()) {
                list = (BasicBSONList) dbc.getNext().get("CataInfo");
            } else {
                return null;
            }
            String srcGroupName = (String) ((BSONObject) list.get(0)).get("GroupName");
            resault.add(srcGroupName);
            if (groups.size() < 2) {
                return resault;
            }
            String destGroupName;
            if (srcGroupName.equals(groups.get(0)))
                destGroupName = groups.get(1);
            else
                destGroupName = groups.get(0);
            resault.add(destGroupName);
            return resault;
        } catch (BaseException e) {
            throw e;
        } finally {
            if (dbc != null) {
                dbc.close();
            }
        }
    }

    public static ArrayList<BSONObject> getReadActList(DBCursor cursor) throws BaseException {
        ArrayList<BSONObject> actRList = new ArrayList<BSONObject>();
        while (cursor.hasNext()) {
            BSONObject record = cursor.getNext();
            actRList.add(record);
        }
        cursor.close();
        return actRList;
    }

    public static ArrayList<BSONObject> insertDatas(DBCollection cl, int startId, int endId, int insertValue)
            throws BaseException {
        ArrayList<BSONObject> insertDatas = new ArrayList<BSONObject>();
        for (int i = startId; i < endId; i++) {
            insertDatas.add((BSONObject) JSON.parse("{_id:" + i + ",a:" + insertValue + ",b:" + i + "}"));
        }
        cl.insert(insertDatas);
        return insertDatas;
    }

    public static ArrayList<BSONObject> insertRandomDatas(DBCollection cl, int startId, int endId)
            throws BaseException {
        ArrayList<BSONObject> insertDatas = new ArrayList<BSONObject>();
        ArrayList<BSONObject> expDatas = new ArrayList<BSONObject>();
        for (int i = startId; i < endId; i++) {
            BSONObject data = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ",b:" + i + "}");
            insertDatas.add(data);
            expDatas.add(data);
        }
        Collections.shuffle(insertDatas);
        cl.insert(insertDatas);
        return expDatas;
    }

    public static boolean getReadActList(DBCursor cursor, StringBuilder expRes, int pos) throws BaseException {
        String prefix = expRes.toString();
        int diff = 1;
        if (pos >= 10) {
            diff = 2;
        }
        while (cursor.hasNext()) {
            BasicBSONObject record = (BasicBSONObject) cursor.getNext();
            String filedA = record.getString("a");
            if (filedA.indexOf(prefix) == -1) {
                return false;
            }

            if (filedA.length() - prefix.length() != diff) {
                return false;
            }

            if (Integer.parseInt(filedA.substring(prefix.length())) != pos) {
                return false;
            }

            pos++;
        }
        return true;
    }

    public static ArrayList<BSONObject> getUpdateDatas(int startId, int endId, int updateValue) {
        ArrayList<BSONObject> updateDatas = new ArrayList<BSONObject>();
        for (int i = startId; i < endId; i++) {
            updateDatas.add((BSONObject) JSON.parse("{_id:" + i + ",a:" + updateValue + ",b:" + i + "}"));
        }
        return updateDatas;
    }

    public static ArrayList<BSONObject> getIncDatas(int startId, int endId, int incValue) {
        ArrayList<BSONObject> incDatas = new ArrayList<BSONObject>();
        for (int i = startId; i < endId; i++) {
            incDatas.add((BSONObject) JSON.parse("{_id:" + i + ",a:" + (incValue + i) + ",b:" + i + "}"));
        }
        return incDatas;
    }

    /**
     * 构造复合索引所需要的数据 如：a:0, b:0 a:1, b:0 a:1, b:1 a:1, b:2 ... a:2, b:2 a:3, b:0 a:3,
     * b:1 ... a 为偶数时，a 和 b 一致 a 为奇数时，有多条记录 a 相等，b 不相等 aStart a 的起始值，aEnd a
     * 的结束值，bStart a 为奇数时 b 的起始值，bEnd a 为奇数时 b 的结束值 返回 list 长度 为 11*(aEnd -
     * aStart)/2
     * 
     * @return
     */
    public static List<BSONObject> getCompositeRecords(int aStart, int aEnd, int bStart, int bEnd) {
        int a = 0;
        int b = 0;
        int id = (aStart / 2) * 11 + aStart % 2;
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = aStart; i < aEnd; i++) {
            if (i % 2 == 0) {
                a = i;
                b = i;
                BSONObject object = (BSONObject) JSON.parse("{_id:" + id++ + ", a:" + a + ", b:" + b + "}");
                records.add(object);
            } else {
                for (int j = bStart; j < bEnd; j++) {
                    a = i;
                    b = j;
                    BSONObject object = (BSONObject) JSON.parse("{_id:" + id++ + ", a:" + a + ", b:" + b + "}");
                    records.add(object);
                }
            }
        }
        Collections.shuffle(records);
        return records;
    }

    /**
     * 排序 参数 key ：true b 字段正序排序，false 逆序
     * 
     * @param records
     */
    public static void sortCompositeRecords(List<BSONObject> records, final boolean key) {

        Collections.sort(records, new Comparator<BSONObject>() {
            @Override
            public int compare(BSONObject obj1, BSONObject obj2) {
                if ((int) obj1.get("a") == (int) obj2.get("a")) {
                    if (key) {
                        return (int) obj1.get("b") - (int) obj2.get("b");
                    } else {
                        return -((int) obj1.get("b") - (int) obj2.get("b"));
                    }
                } else {
                    return (int) obj1.get("a") - (int) obj2.get("a");
                }
            }
        });
    }
}