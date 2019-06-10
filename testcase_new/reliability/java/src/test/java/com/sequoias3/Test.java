package com.sequoias3;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import org.bson.BSONObject;

/**
 * Created by fanyu on 2019/6/6.
 */

public class Test{
    public static void main(String[] args) throws Exception {
        Sequoiadb db = new Sequoiadb("192.168.31.52:11810","sdbadmin","sequoiadb");
        DBCursor cursor = db.listCollectionSpaces();
        while (cursor.hasNext()){
            BSONObject bson = cursor.getNext();
           if(!bson.get("Name").equals("S3_SYS_Meta")){
               db.dropCollectionSpace((String)bson.get("Name"));
           }
        }
    }

}
