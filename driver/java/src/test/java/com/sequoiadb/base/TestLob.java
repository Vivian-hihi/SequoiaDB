package com.sequoiadb.base;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.test.SingleCSCLTestCase;
import com.sequoiadb.test.TestConfig;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.junit.Before;
import org.junit.Test;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.Random;

import static org.junit.Assert.*;

public class TestLob extends SingleCSCLTestCase {
    @Before
    public void setUp() {
        cl.truncate();
    }

    @Test
    public void testLob() {
        String str = "Hello, world!";

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.write(str.getBytes());
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s = new String(bytes);
        assertEquals(str, s);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLob2() {
        int bytesNum = 1024 * 1024;
        byte[] bytes = new byte[bytesNum];
        Random rand = new Random();
        rand.nextBytes(bytes);

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.write(bytes);
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes2 = new byte[(int) lob.getSize()];
        lob.read(bytes2);
        lob.close();

        assertTrue(Arrays.equals(bytes, bytes2));

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLob3() {
        int bytesNum = 1000 * 1000 + 3000;
        int step = 3003;
        int offset = 1024 * 2;
        byte[] bytes = new byte[bytesNum];
        Random rand = new Random();
        rand.nextBytes(bytes);

        SimpleDateFormat df = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS");

        for (int length = step, count = 0; length + step < bytes.length; length += step, count++) {
            ObjectId id = ObjectId.get();
            DBLob lob = cl.createLob(id);
            lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
            lob.write(bytes, 0, length);
            lob.close();

            long lobSize = lob.getSize();

            DBCursor cursor = cl.listLobs();
            assertTrue(cursor.hasNext());
            BSONObject obj = cursor.getNext();
            ObjectId oid = (ObjectId) obj.get("Oid");
            assertEquals(id, oid);
            assertFalse(cursor.hasNext());

            lob = cl.openLob(id);
            assertEquals(lobSize, lob.getSize());
            assertEquals(lobSize, offset + length);
            byte[] bytesRead = new byte[(int) lob.getSize()];
            lob.read(bytesRead);
            lob.close();

            byte[] bytes1 = new byte[length];
            System.arraycopy(bytesRead, offset, bytes1, 0, length);

            byte[] bytes2 = new byte[length];
            System.arraycopy(bytes, 0, bytes2, 0, length);

            assertTrue(Arrays.equals(bytes2, bytes1));

            cl.removeLob(id);
            cursor = cl.listLobs();
            assertFalse(cursor.hasNext());

            if (count % 100 == 0) {
                System.out.println(count + ": " + df.format(new Date()) + ": length=" + length);
            }
        }
    }

    @Test
    public void testLobSeekWrite() {
        String str = "Hello, world!";
        String str2 = "LOB random write";
        int offset = 10;

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
        lob.write(str.getBytes());
        lob.write(str2.getBytes());
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s = new String(bytes, offset, bytes.length - offset);
        assertEquals(str + str2, s);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobSeekWrite2() {
        String str = "Hello, world!";
        String str2 = "LOB seek write";
        int offset = 100;
        int offset2 = 10;

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
        lob.write(str.getBytes());
        lob.seek(offset2, DBLob.SDB_LOB_SEEK_SET);
        lob.write(str2.getBytes());
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s = new String(bytes, offset, bytes.length - offset);
        assertEquals(str, s);

        String s2 = new String(bytes, offset2, str2.length());
        assertEquals(str2, s2);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobSeekWrite3() {
        String str = "Hello, world!";
        String str2 = "LOB random write";
        int offset = 256 * 1024;

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
        lob.write(str.getBytes());
        lob.write(str2.getBytes());
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s = new String(bytes, offset, bytes.length - offset);
        assertEquals(str + str2, s);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobSeekWrite4() {
        String str = "Hello, world!";
        String str2 = "LOB seek write";
        int offset = 256 * 1024 * 2;
        int offset2 = 256 * 1024 * 4;

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
        lob.write(str.getBytes());
        lob.seek(offset2, DBLob.SDB_LOB_SEEK_SET);
        lob.write(str2.getBytes());
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s = new String(bytes, offset, str.length());
        assertEquals(str, s);

        String s2 = new String(bytes, offset2, str2.length());
        assertEquals(str2, s2);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobSeekWrite5() {
        int bytesNum = 1024 * 1024 * 2;
        byte[] bytes = new byte[bytesNum];
        Random rand = new Random();
        rand.nextBytes(bytes);

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.seek(1024 * 256 * 2, DBLob.SDB_LOB_SEEK_SET);
        lob.write(bytes);
        lob.seek(1024 * 256, DBLob.SDB_LOB_SEEK_SET);
        lob.write(bytes);
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes2 = new byte[bytesNum];
        lob.seek(1024 * 256, DBLob.SDB_LOB_SEEK_SET);
        lob.read(bytes2);
        lob.close();

        assertArrayEquals(bytes, bytes2);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobSeekWrite6() {
        String str = "Hello, world!";
        byte[] bytes = str.getBytes();

        int begin = 1024 * 3 + 11;
        int step = 1024 * 4 * 2;
        int max = 1024 * 256;
        ArrayList<Integer> posList = new ArrayList<>();
        for (int pos = begin; pos <= max; pos += step) {
            posList.add(pos);
        }

        Random rand = new Random(System.currentTimeMillis());
        ArrayList<Integer> writePos = new ArrayList<>(posList);

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        while (!writePos.isEmpty()) {
            int index = rand.nextInt(writePos.size());
            int pos = writePos.remove(index);
            lob.seek(pos, DBLob.SDB_LOB_SEEK_SET);
            lob.write(bytes);
        }
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());

        ArrayList<Integer> readPos = new ArrayList<>(posList);
        while (!readPos.isEmpty()) {
            int index = rand.nextInt(readPos.size());
            int pos = readPos.remove(index);
            lob.seek(pos, DBLob.SDB_LOB_SEEK_SET);
            byte[] bytes2 = new byte[str.length()];
            lob.read(bytes2);
            String str2 = new String(bytes2);
            assertEquals(str, str2);
        }
        lob.close();

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobOpenWrite() {
        String str = "Hello, world!";

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.close();

        lob = cl.openLob(id, DBLob.SDB_LOB_WRITE);
        lob.write(str.getBytes());
        lob.lock(0, -1);
        lob.close();

        long lobSize = lob.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s = new String(bytes);
        assertEquals(str, s);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    @Test
    public void testLobOpenWrite2() {
        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.close();

        lob = cl.openLob(id, DBLob.SDB_LOB_WRITE);
        lob.lock(100, 5);
        lob.lock(90, 5);
        lob.lock(80, 5);
        lob.lock(115, 5);
        lob.lock(110, 10);
        lob.lock(112, 5);
        lob.lock(75, 10);
        lob.lock(87, 20);
        lob.close();

        lob = cl.openLob(id, DBLob.SDB_LOB_WRITE);
        lob.lock(100, 5);
        lob.lock(90, 5);
        lob.lock(80, 5);
        lob.lock(115, 5);
        lob.lock(110, 10);
        lob.lock(112, 5);
        lob.lock(75, 10);
        lob.lock(87, 20);
        lob.close();
    }

    @Test
    public void testLobOpenWrite3() {
        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.close();

        DBLob lob1 = cl.openLob(id, DBLob.SDB_LOB_WRITE);
        DBLob lob2 = cl.openLob(id, DBLob.SDB_LOB_WRITE);

        lob1.lock(100, 10);

        lob2.lock(90, 10);
        lob2.lock(110, 10);
        try {
            lob2.lock(90, 20);
            fail("failure expected");
        } catch (BaseException e) {
            System.out.println(e);
        }
        try {
            lob2.lock(105, 10);
            fail("failure expected");
        } catch (BaseException e) {
            System.out.println(e);
        }

        lob1.close();

        lob2.lock(90, 20);
        lob2.lock(105, 10);
        lob2.close();
    }

    @Test
    public void testLobOpenWrite4() {
        String str1 = "Hello, world!";
        int offset1 = 1024 * 256 * 2 + 1024 * 2;
        String str2 = "LOB random write";
        int offset2 = offset1 - str2.length() * 2;

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.close();

        DBLob lob1 = cl.openLob(id, DBLob.SDB_LOB_WRITE);
        DBLob lob2 = cl.openLob(id, DBLob.SDB_LOB_WRITE);

        lob1.lockAndSeek(offset1, str1.length());
        lob1.write(str1.getBytes());

        lob2.lockAndSeek(offset2, str2.length());
        lob2.write(str2.getBytes());

        lob1.close();
        lob2.close();

        long lobSize = lob1.getSize();

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(lobSize, lob.getSize());
        byte[] bytes = new byte[(int) lob.getSize()];
        lob.read(bytes);
        lob.close();

        String s1 = new String(bytes, offset1, str1.length());
        assertEquals(str1, s1);

        String s2 = new String(bytes, offset2, str2.length());
        assertEquals(str2, s2);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }

    class LobWriter implements Runnable {
        private int index;
        private String csName;
        private String clName;
        private ObjectId id;
        private byte[] data;
        private int offset;
        private int length;

        public LobWriter(int index, String csName, String clName, ObjectId id, byte[] data, int offset, int length) {
            this.index = index;
            this.csName = csName;
            this.clName = clName;
            this.id = id;
            this.data = data;
            this.offset = offset;
            this.length = length;
        }

        @Override
        public void run() {
            try (Sequoiadb sdb = new Sequoiadb(TestConfig.getSingleHost(),
                Integer.valueOf(TestConfig.getSinglePort()),
                TestConfig.getSingleUsername(),
                TestConfig.getSinglePassword())) {
                DBCollection cl = sdb.getCollectionSpace(csName)
                    .getCollection(clName);
                try (DBLob lob = cl.openLob(id, DBLob.SDB_LOB_WRITE)) {
                    lob.lockAndSeek(offset, length);
                    lob.write(data, offset, length);
                }
            } catch (Exception e) {
                e.printStackTrace();
                throw e;
            }
            System.out.println(
                String.format("Thread[%d]: offset=%d, length=%d",
                    index, offset, length));
        }
    }

    @Test
    public void testConcurrentWrite() {
        int bytesNum = 1024 * 1024 * 5;
        byte[] bytes = new byte[bytesNum];
        Random rand = new Random();
        rand.nextBytes(bytes);

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        lob.close();

        int threadNum = 7;
        Thread[] threads = new Thread[threadNum];
        int offset = 0;
        for (int i = 0; i < threadNum; i++) {
            int length;
            if (i < threadNum - 1) {
                length = bytesNum / threadNum;
            } else {
                length = bytesNum - offset;
            }
            threads[i] = new Thread(
                new LobWriter(i, cl.getCSName(), cl.getName(), id, bytes, offset, length));
            offset += length;
        }

        PerfTimer timer = new PerfTimer();
        timer.start();

        for (Thread thread : threads) {
            thread.start();
        }

        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }

        timer.stop();
        System.out.println(
            String.format("Concurrent writing lob costs %dms in %d thread(s)",
                timer.duration(), threadNum));

        DBCursor cursor = cl.listLobs();
        assertTrue(cursor.hasNext());
        BSONObject obj = cursor.getNext();
        ObjectId oid = (ObjectId) obj.get("Oid");
        assertEquals(id, oid);
        assertFalse(cursor.hasNext());

        lob = cl.openLob(id);
        assertEquals(bytesNum, lob.getSize());
        byte[] bytes2 = new byte[(int) lob.getSize()];
        lob.read(bytes2);
        lob.close();

        assertArrayEquals(bytes, bytes2);

        cl.removeLob(id);
        cursor = cl.listLobs();
        assertFalse(cursor.hasNext());
    }
}
