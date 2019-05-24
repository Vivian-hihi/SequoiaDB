package com.sequoiadb.transaction;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.exception.BaseException;

public class TransactionUtils {
    public static void insertData(DBCollection cl,int start, int recSum, int strLength){
        try {
            List<BSONObject> recs = new ArrayList<BSONObject>();
            for(int i = start; i < recSum; i++){
                BSONObject rec = new BasicBSONObject();
                rec.put("_id", i);
                rec.put("age", i);
                rec.put("num", i);
                rec.put("str", getRandomString(strLength));
                recs.add(rec);
            }
           cl.insert(recs);
        } catch (BaseException e) {
            throw e;
        }
    }

    private static String getRandomString(int length){
        String base = "abc";
        Random random = new Random();
        StringBuffer sb = new StringBuffer();
        for(int i = 0; i < length; i++){
            int index = random.nextInt(base.length());
            sb.append(base.charAt(index));
        }
        return sb.toString();
    }
}
