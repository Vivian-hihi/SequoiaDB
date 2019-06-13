package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;

/**
 * ES的公共类，涉及ES内部操作的方法放于此类
 */
public class FullTextESUtils {

    /**
     * 获取elasticsearch端全文索引的总记录数
     * 
     * @param esClient
     * @param esIndexName
     * @return long 返回记录总数
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static int getCountFromES(String esIndexName) throws Exception {
        FullTextRest rest = new FullTextRest();
        ResponseEntity<?> response = null;
        try {
            response = rest.setApi("/" + esIndexName + "/_count").setRequestMethod(HttpMethod.GET)
                    .setResponseType(String.class).exec();
        } catch (Exception e) {
            if (e.getMessage().equals("404 Not Found")) {
                throw new Exception("no such index");
            } else {
                throw e;
            }
        }
        String body = response.getBody().toString();
        BSONObject bodyObj = (BSONObject) JSON.parse(body);

        // 减去SDBCOMMITID记录
        return (int) bodyObj.get("count") - 1;
    }

    /**
     * 获取elasticsearch端的SDBCOMMIT记录下的逻辑ID值
     * 
     * @param esClient
     * @param esIndexName
     * @return int 返回SDBCOMMIT._lid值
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    @SuppressWarnings("unchecked")
    public static int getCommitIDFromES(String esIndexName) throws Exception {
        int commitID = -1;

        FullTextRest rest = new FullTextRest();
        ResponseEntity<?> response = null;
        try {
            response = rest.setApi("/" + esIndexName + "/_search?q=_id:SDBCOMMIT").setRequestMethod(HttpMethod.GET)
                    .setResponseType(String.class).exec();
        } catch (Exception e) {
            if (e.getMessage().equals("404 Not Found")) {
                throw new Exception("no such index");
            } else {
                throw e;
            }
        }
        String body = response.getBody().toString();
        BSONObject bodyObj = (BSONObject) JSON.parse(body);
        BSONObject hitss = (BSONObject) bodyObj.get("hits");
        if ((int) hitss.get("total") == 0) {
            throw new Exception("no such _id=SDBCOMMIT record");
        }
        BSONObject hits = ( (List<BSONObject>) hitss.get("hits") ).get(0);
        BSONObject source = (BSONObject) hits.get("_source");
        commitID = (int) source.get("_lid");

        return commitID;
    }

    /**
     * 获取elasticsearch端的SDBCOMMIT记录下的原始集合逻辑ID值
     * 
     * @param esClient
     * @param esIndexName
     * @return int 返回SDBCOMMIT._cllid值
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2019-05-16
     */
    @SuppressWarnings("unchecked")
    public static int getCommitCLLIDFromES(String esIndexName) throws Exception {
        int commitCLLID = -1;

        FullTextRest rest = new FullTextRest();
        ResponseEntity<?> response = null;
        try {
            response = rest.setApi("/" + esIndexName + "/_search?q=_id:SDBCOMMIT").setRequestMethod(HttpMethod.GET)
                    .setResponseType(String.class).exec();
        } catch (Exception e) {
            if (e.getMessage().equals("404 Not Found")) {
                throw new Exception("no such index");
            } else {
                throw e;
            }
        }
        String body = response.getBody().toString();
        BSONObject bodyObj = (BSONObject) JSON.parse(body);
        BSONObject hitss = (BSONObject) bodyObj.get("hits");
        if ((int) hitss.get("total") == 0) {
            throw new Exception("no such index");
        }
        BSONObject hits = ( (List<BSONObject>) hitss.get("hits") ).get(0);
        BSONObject source = (BSONObject) hits.get("_source");
        commitCLLID = (int) source.get("_cllid");

        return commitCLLID;
    }

    /**
     * 获取多个elasticsearch端的SDBCOMMIT记录下的原始集合逻辑ID值
     * 
     * @param esClient
     * @param esIndexNames
     * @return List < Integer > 返回每个全文索引的SDBCOMMIT._cllid值
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2019-05-17
     */
    public static List<Integer> getCommitCLLIDFromES(List<String> esIndexNames) throws Exception {
        List<Integer> commitCLLIDs = new ArrayList<>();

        for (String esIndexName : esIndexNames) {
            commitCLLIDs.add(getCommitCLLIDFromES(esIndexName));
        }

        return commitCLLIDs;
    }

    /**
     * 判断elasticsearch端的全文索引名是否存在，用于检查在创建阶段索引名是否映射到elasticsearch端
     * 
     * @param esClient
     * @param esIndexName
     * @return boolean 索引已在ES创建成功则返回true,否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static boolean isIndexCreatedInES(String esIndexName) throws Exception {
        return isExistIndexInES(esIndexName, true);
    }

    /**
     * 判断elasticsearch端的全文索引名是否被删除，用于清理阶段作环境检查
     * 
     * @param esClient
     * @param esIndexName
     * @return boolean 索引已在ES创建删除则返回true,否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public boolean isIndexDeletedInES(String esIndexName) throws Exception {
        return !isExistIndexInES(esIndexName, false);
    }

    /**
     * 判断elasticsearch端的全文索引名是否被删除，用于清理阶段作环境检查
     * 
     * @param esClient
     * @param esIndexNames
     * @return boolean 索引已在ES创建删除则返回true,否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public boolean isIndexDeletedInES(List<String> esIndexNames) throws Exception {
        boolean notExist = true;
        for (String esIndexName : esIndexNames) {
            notExist = isExistIndexInES(esIndexName, false);
            if (notExist) {
                break;
            }
        }
        return !notExist;
    }

    /**
     * 判断elasticsearch端的全文索引名是否存在
     * 
     * @param esClient
     * @param esIndexName
     * @param expExist
     * @return boolean 存在返回true, 否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static boolean isExistIndexInES(String esIndexName, boolean expExist) throws Exception {
        int timeout = 600; // 超时时间600s
        int doTimes = 0;
        int interval = 1;
        boolean indexExist = false;

        FullTextRest rest = new FullTextRest();
        while (doTimes * interval < timeout) {
            ResponseEntity<?> response = null;
            try {
                response = rest.setApi("/" + esIndexName).setRequestMethod(HttpMethod.GET).setResponseType(String.class)
                        .exec();
            } catch (Exception e) {
                if (e.getMessage().equals("404 Not Found")) {
                    indexExist = false;
                } else {
                    throw e;
                }
            }
            if (response != null && response.getStatusCodeValue() == 200) {
                indexExist = true;
            }

            if (expExist != indexExist) {
                doTimes++;
                // 每次循环间隔1s
                try {
                    Thread.sleep(interval * 1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            } else {
                // 防止读到旧数据,由于es端每隔5s刷新一次,因此这里需要等待6s
                try {
                    Thread.sleep(6000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                break;
            }
        }

        if (indexExist != expExist) {
            String msg = expExist ? "es client no such index: " : "index is still in the es: ";
            throw new Exception(msg + esIndexName);
        }

        return indexExist;
    }

}
