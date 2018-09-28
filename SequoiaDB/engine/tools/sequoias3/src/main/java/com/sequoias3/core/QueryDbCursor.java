package com.sequoias3.core;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;

public class QueryDbCursor {
    private Sequoiadb sdb;
    private DBCursor cursor;

    public QueryDbCursor(Sequoiadb sdb, DBCursor cursor){
        this.sdb = sdb;
        this.cursor = cursor;
    }

    public void setSdb(Sequoiadb sdb) {
        this.sdb = sdb;
    }

    public Sequoiadb getSdb() {
        return sdb;
    }

    public void setCursor(DBCursor cursor) {
        this.cursor = cursor;
    }

    public DBCursor getCursor() {
        return cursor;
    }
}
