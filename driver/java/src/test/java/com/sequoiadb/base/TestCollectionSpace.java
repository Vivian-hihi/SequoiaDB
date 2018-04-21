package com.sequoiadb.base;

import com.sequoiadb.test.SingleTestCase;
import org.bson.types.ObjectId;
import org.junit.Test;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import static org.junit.Assert.*;

public class TestCollectionSpace extends SingleTestCase {
    private String csName;

    public void setUp() {
        csName = "TestCollectionSpace_" + new ObjectId().toString();
    }

    public void tearDown() {
        csName = null;
    }

    @Test
    public void testCreateDrop() {
        assertFalse(sdb.isCollectionSpaceExist(csName));

        CollectionSpace cs = sdb.createCollectionSpace(csName);
        assertEquals(csName, cs.getName());
        assertEquals(sdb, cs.getSequoiadb());
        assertEquals(0, cs.getCollectionNames().size());

        assertTrue(sdb.isCollectionSpaceExist(csName));
        sdb.dropCollectionSpace(csName);
        assertFalse(sdb.isCollectionSpaceExist(csName));
    }

    @Test
    public void testAlter() {
        assertFalse(sdb.isCollectionSpaceExist(csName));

        CollectionSpace cs = sdb.createCollectionSpace(csName);
        assertEquals(csName, cs.getName());
        assertEquals(sdb, cs.getSequoiadb());
        assertEquals(0, cs.getCollectionNames().size());

        BSONObject options = new BasicBSONObject();
        options.put("LobPageSize",8192);
        cs.alterCollectionSpace(options);

        assertTrue(sdb.isCollectionSpaceExist(csName));
        sdb.dropCollectionSpace(csName);
        assertFalse(sdb.isCollectionSpaceExist(csName));
    }

    @Test
    public void testSetAttributes() {
        assertFalse(sdb.isCollectionSpaceExist(csName));

        CollectionSpace cs = sdb.createCollectionSpace(csName);
        assertEquals(csName, cs.getName());
        assertEquals(sdb, cs.getSequoiadb());
        assertEquals(0, cs.getCollectionNames().size());

        BSONObject options = new BasicBSONObject();
        options.put("LobPageSize",8192);
        cs.setAttributes(options);

        assertTrue(sdb.isCollectionSpaceExist(csName));
        sdb.dropCollectionSpace(csName);
        assertFalse(sdb.isCollectionSpaceExist(csName));
    }
}
