package com.sequoiadb.base;

import com.sequoiadb.test.SingleCSCLTestCase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;

import java.io.UnsupportedEncodingException;
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

    @Test @Ignore
    public void testLob3() {
        int bytesNum = 1000 * 1000 + 3000;
        int step = 1001;
        int offset = 1024 * 2 ;
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
        int max = 1024 * 1024;
        ArrayList<Integer> posList = new ArrayList<>();
        for (int pos = begin; pos <= max; pos += step) {
            posList.add(pos);
        }

        Random rand = new Random(System.currentTimeMillis());
        ArrayList<Integer> writePos = new ArrayList<>(posList);

        ObjectId id = ObjectId.get();
        DBLob lob = cl.createLob(id);
        while (!writePos.isEmpty()){
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
        while (!readPos.isEmpty()){
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
}
