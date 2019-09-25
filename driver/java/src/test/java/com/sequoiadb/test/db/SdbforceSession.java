package com.sequoiadb.test.db;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.test.common.Constants;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;
import org.junit.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import static com.sequoiadb.base.Sequoiadb.SDB_SNAP_SESSIONS;
import static com.sequoiadb.base.Sequoiadb.SDB_SNAP_SESSIONS_CURRENT;
import static org.junit.Assert.assertTrue;

public class SdbforceSession {

    private static Sequoiadb sdb;
    private static CollectionSpace cs;
    private static DBCollection cl;
    private static ReplicaGroup rg;
    private static Node node;
    private static DBCursor cursor;
    private static boolean isCluster = true;

    @BeforeClass
    public static void setConnBeforeClass() throws Exception {
        isCluster = Constants.isCluster();
        // sdb
        sdb = new Sequoiadb(Constants.COOR_NODE_CONN, "", "");

        //sdb = new Sequoiadb("192.168.20.50:50000", "", "");
    }

    @AfterClass
    public static void DropConnAfterClass() throws Exception {
        sdb.disconnect();
    }

    @Before
    public void setUp() throws Exception {
        // cs
        if (sdb.isCollectionSpaceExist(Constants.TEST_CS_NAME_1)) {
            sdb.dropCollectionSpace(Constants.TEST_CS_NAME_1);
            cs = sdb.createCollectionSpace(Constants.TEST_CS_NAME_1);
        } else
            cs = sdb.createCollectionSpace(Constants.TEST_CS_NAME_1);
        // cl
        BSONObject conf = new BasicBSONObject();
        conf.put("ReplSize", 0);
        cl = cs.createCollection(Constants.TEST_CL_NAME_1, conf);
    }

    @After
    public void tearDown() throws Exception {
        sdb.dropCollectionSpace(Constants.TEST_CS_NAME_1);
    }

    public long getCurrentSessionID(){
        long sessionID = -1;
        BSONObject con = new BasicBSONObject();
        con.put("Global",false);
        DBCursor cursor = sdb.getSnapshot(SDB_SNAP_SESSIONS_CURRENT, con, null, null);

        if (cursor.hasNext()){
            BSONObject obj = cursor.getNext();
            sessionID = (Long)obj.get("SessionID");
            System.out.println("SessionID:" + sessionID);
        }
        cursor.close();
        return sessionID;
    }
    @Test
    public void forceSession_test() {
        long sessionID = getCurrentSessionID();
        if ( sessionID != -1){
            sdb.forceSession(sessionID);
        }
    }

    @Test
    public void forceSession_test_with_options() {
        BSONObject options = new BasicBSONObject();
        options.put("Global",false);
        options.put("GroupName","db1");
        long sessionID = getCurrentSessionID();
        if ( sessionID != -1){
            sdb.forceSession(sessionID, options);
        }
    }

}
