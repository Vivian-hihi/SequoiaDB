//package com.sequoiadb.lob;
//
//import com.sequoiadb.base.DBCollection;
//import com.sequoiadb.base.Sequoiadb;
//import com.sequoiadb.commlib.GroupMgr;
//import com.sequoiadb.commlib.StandTestInterface;
//import com.sequoiadb.exception.BaseException;
//import com.sequoiadb.exception.ReliabilityException;
//import com.sequoiadb.fault.BrokenNetwork;
//import com.sequoiadb.metaopr.commons.MyUtil;
//import com.sequoiadb.task.FaultMakeTask;
//import com.sequoiadb.task.TaskMgr;
//import org.bson.BSONObject;
//import org.bson.BasicBSONObject;
//import org.bson.types.ObjectId;
//import org.testng.Assert;
//import org.testng.annotations.AfterClass;
//import org.testng.annotations.BeforeClass;
//import org.testng.annotations.BeforeTest;
//import org.testng.annotations.Test;
//
//import java.util.ArrayList;
//import java.util.Arrays;
//import java.util.HashMap;
//import java.util.List;
//
//import static com.sequoiadb.metaopr.commons.MyUtil.*;
//import static org.testng.AssertJUnit.assertTrue;
//
///**
// * @FileName
// * @Author laojingtang
// * @Date 17-5-11
// * @Version 1.00
// */
//public class LobTemp implements StandTestInterface {
//    String csName = "lobcs";
//    String clName = "bar";
//    //    private byte[] data = createRandomBytes(200 * 1024);
//    private List<ObjectId> ids = new ArrayList<>();
//
//    private List<byte[]> datas = new ArrayList<>(1000);
//
//
//    @BeforeTest
//    @Override
//    public void setup() {
//        dropCS("lobcs");
//        Sequoiadb db=getSdb();
//        int size=4096*2*2*4;
//        BSONObject clOption=new BasicBSONObject();
//        clOption.put("ReplSize",3);
//        clOption.put("Group","group1");
//
//        db.createCollectionSpace("lobcs").createCollection("bar",new BasicBSONObject("ReplSize",3));
//
//
//        for (int i = 1; i <1000; i++) {
//            datas.add(createRandomBytes(size+1));
//        }
//    }
//
//    @AfterClass
//    @Override
//    public void tearDown() {
//        MyUtil.printEndTime(this);
//    }
//
//    @Test(invocationCount = 1)
//    public void test() throws ReliabilityException, InterruptedException {
//
//        List<ObjectId> createIDs = MyUtil.createLob(csName, clName, datas);
//
////        HashMap<ObjectId, String> deletedIdMap = new HashMap<>();
//        LobTask readTask = LobTask.getReadLobsTask(ids);
////        LobTask deleteTask = LobTask.getDeleteLobsTask(ids, deletedIdMap);
//
//
//        int count = 0;
//        while (!MyUtil.isLobsAllCreated(csName, clName, createIDs)) {
//            Thread.sleep(1000);
//            count++;
//            if (count > 5) {
//                Assert.fail();
//                System.exit(0);
//            }
//        }
////        assertTrue(MyUtil.isLobNumInspectInGroup(csName, clName, "group1"));
////        assertTrue(MyUtil.isLobNumInspectInGroup(csName, clName, "group2"));
////        assertTrue(MyUtil.isLobMd5InspectInGroup(csName, clName, "group1", targetMd5Value));
////        assertTrue(MyUtil.isLobMd5InspectInGroup(csName, clName, "group2", targetMd5Value));
//
//        MyUtil.checkBusiness();
//        Sequoiadb db = getSdb();
//        DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
//
//        for (int i = 0; i < createIDs.size(); i++) {
//
//            int inc = 0;
//            byte[] lobbyte;
//            while (true) {
//                if (inc > 5) {
//                    Assert.fail();
//                    System.exit(0);
//                }
//                try {
//                    lobbyte = readLob(cl, createIDs.get(i));
//                    break;
//                } catch (BaseException e) {
//                    e.printStackTrace();
//                    inc++;
//                }
//            }
//
//
//            byte[] targetMd5Value = getMd5(datas.get(i));
//            boolean result = Arrays.equals(getMd5(lobbyte), targetMd5Value);
//            if (result == false)
//                Assert.fail(createIDs.get(i).toString() + " targetMd5Value: " + targetMd5Value + " lobmd5" + getMd5(lobbyte));
//        }
//        try {
//            MyUtil.deleteAllLobs(csName, clName);
//        }catch (BaseException e)
//        {
//            e.printStackTrace();
//            System.exit(0);
//        }
//    }
//}
