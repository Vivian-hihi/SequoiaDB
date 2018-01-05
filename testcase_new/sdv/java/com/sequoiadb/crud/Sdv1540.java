package com.sequoiadb.crud;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.types.ObjectId;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;

import static org.testng.Assert.*;

/**
 * Created by laojingtang on 18-1-4.
 */
public class Sdv1540 extends SdbTestBase {
    private Sequoiadb db = null;
    private static final String CLNAME = Sdv1540.class.getSimpleName();
    private DBCollection dbcl;
    private List<String> coorURLs = new ArrayList<>();
    private Random random = new Random();

    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        CollectionSpace cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(CLNAME);

        //get all coord node url
        BSONObject obj = db.getReplicaGroup("SYSCoord").getDetail();
        BasicBSONList group = (BasicBSONList) obj.get("Group");
        for (Object o : group) {
            BasicBSONObject node = (BasicBSONObject) o;
            String hostName = node.getString("HostName");
            String path = node.getString("dbpath");
            String[] s = path.split("/");
            String port = s[s.length - 1];
            coorURLs.add(hostName + ":" + port);
        }
    }

    @AfterClass
    public void teardown() {
        if (db != null) {
            db.getCollectionSpace(SdbTestBase.csName).dropCollection(CLNAME);
            db.disconnect();
        }
    }

    /**
     * 1、cl中插入记录和lob
     * 2、并发连接到不同的coord执行如下并发操作：
     * 1）并发插入数据
     * 2）并发修改记录
     * 3）并发删除记录（删除已经插入成功的记录）
     * 4）并发查询记录
     * 5）并发插入lob
     * 6）并发删除lob（删除已经插入成功的lob）
     * 7）并发读取lob
     * 8）创建索引
     * 3、检查各操作结果
     */
    @Test
    public void test() {
        prepareSimpleData();

        MyTask insert = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                cl.insert(Arrays.asList(genrateData(1000)));
            }
        };
        MyTask delete = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                for (int i = 0; i < 100; i++) {
                    BasicBSONObject o = (BasicBSONObject) cl.queryOne();
                    if (o == null)
                        continue;
                    cl.delete(new BasicBSONObject("_id", o.getObjectId("_id")));
                }
            }
        };
        MyTask update = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                for (int i = 0; i < 100; i++) {
                    BasicBSONObject o = (BasicBSONObject) cl.queryOne();
                    if (o == null)
                        continue;
                    o.getObjectId("_id");
                    cl.queryAndUpdate(new BasicBSONObject("_id", o.getObjectId("_id"))
                            , new BasicBSONObject()
                            , new BasicBSONObject()
                            , new BasicBSONObject()
                            , new BasicBSONObject("$inc", new BasicBSONObject("a", 1)), 0, 10, 0, true);
                }
            }
        };
        MyTask query = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                for (int i = 0; i < 100; i++) {
                    cl.query("", "", "", "", 0, 10).close();
                }
            }
        };
        MyTask createLob = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                for (int i = 0; i < 10; i++) {
                    DBLob lob = cl.createLob();
                    lob.write(data);
                    lob.close();
                }
            }
        };
        MyTask readLob = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                List<ObjectId> ids = getLobOids(cl);
                for (int i = 0; i < 10; i++) {
                    ObjectId id = ids.get(random.nextInt(ids.size()));
                    try {
                        DBLob lob = cl.openLob(id);
                        lob.read(new byte[(int) lob.getSize()]);
                        lob.close();
                    } catch (BaseException e) {
                        if (e.getErrorCode() != -4) {
                            throw e;
                        }
                    }
                }
            }
        };
        MyTask deleteLob = new MyTask() {
            @Override
            void opration(DBCollection cl) {
                List<ObjectId> ids = getLobOids(cl);
                for (int i = 0; i < 10; i++) {
                    ObjectId id = ids.get(random.nextInt(ids.size()));
                    try {
                        cl.removeLob(id);
                    } catch (BaseException e) {
                        //-4 when the lob maybe already removed throw -4
                        //-137 when the lob is in use throw -137
                        if (e.getErrorCode() != -4 && e.getErrorCode() != -137) {
                            throw e;
                        }
                    }
                }
            }
        };

        List<MyTask> tasks = new ArrayList<>(10);
        tasks.add(insert);
        tasks.add(delete);
        tasks.add(update);
        tasks.add(query);
        tasks.add(createLob);
        tasks.add(deleteLob);
        tasks.add(readLob);

        insert.start(10);
        delete.start(10);
        update.start(10);
        query.start(10);
        createLob.start(10);
        readLob.start(10);
        deleteLob.start(10);

        for (MyTask task : tasks) {
            assertTrue(task.isSuccess(), task.getErrorMsg());
        }
    }

    private List<ObjectId> getLobOids(DBCollection cl) {
        DBCursor cur = cl.listLobs();
        List<ObjectId> ids = new ArrayList<>(1000);
        while (cur.hasNext()) {
            BasicBSONObject o = (BasicBSONObject) cur.getNext();
            ObjectId id = o.getObjectId("Oid");
            ids.add(id);
        }
        return ids;
    }

    abstract class MyTask extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(getCoordURLRandom(), "", "");
                DBCollection cl = db.getCollectionSpace(SdbTestBase.csName)
                        .getCollection(CLNAME);
                opration(cl);
            } finally {
                if (db != null)
                    db.disconnect();
            }
        }

        abstract void opration(DBCollection cl);
    }

    private String getCoordURLRandom() {
        return coorURLs.get(random.nextInt(coorURLs.size()));
    }

    private byte[] data = new byte[1024];

    private void prepareSimpleData() {
        dbcl.insert(Arrays.asList(genrateData(10000)));
        for (int i = 0; i < 10000; i++) {
            DBLob lob = dbcl.createLob();
            lob.write(data);
            lob.close();
        }
    }

    private BSONObject[] genrateData(int num) {
        BSONObject[] b = new BSONObject[num];
        for (int i = 0; i < num; i++) {
            b[i] = new BasicBSONObject("a", i).append("b", i).append("c", i);
        }
        return b;
    }
}
