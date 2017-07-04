package com.sequoiadb.lob;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import org.bson.BSONObject;
import org.bson.util.JSON;

import static com.sequoiadb.metaopr.commons.MyUtil.*;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-5-11
 * @Version 1.00
 */
public class LobUtil {
    public static final String csName = "lobcs", clName = "lobcl";


    public static void createLobCsAndCl() {
        Sequoiadb db = getSdb();
        if (isCsExisted(csName) == true) {
            if (db.getCollectionSpace(csName).getCollection(clName) != null)
                return;
        }
        createCS(csName);
        BSONObject option = (BSONObject) JSON.parse("{ ShardingKey: { \"age\": 1 }," +
                " ShardingType: \"hash\", " +
                "Partition: 1024, ReplSize: 1," +
                " Compressed: true ," +
                "Group:\"group1\"}");
        createCl(csName, clName, option);
        DBCollection cl = db.getCollectionSpace(csName)
                .getCollection(clName);

        cl.split("group1", "group2", 50);
        db.close();
    }

    public static void dropLobCS(){
        Sequoiadb db=getSdb();
        try {
            db.dropCollectionSpace(csName);
        }catch (BaseException e){}
        db.close();
    }

}
