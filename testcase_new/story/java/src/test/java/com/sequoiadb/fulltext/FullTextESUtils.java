package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.net.InetAddress;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.elasticsearch.action.admin.indices.exists.indices.IndicesExistsRequest;
import org.elasticsearch.action.admin.indices.exists.indices.IndicesExistsResponse;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.action.search.SearchType;
import org.elasticsearch.client.*;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.transport.client.PreBuiltTransportClient;
import org.elasticsearch.index.query.*;
import org.elasticsearch.search.SearchHit;

public class FullTextESUtils {

    public static final String ES_HOSTNAME = "192.168.31.42";
    public static final int ES_SVCNAME = 9300; //cannot be the elasticsearch port
	
    @SuppressWarnings("resource")
    private static Client createTransportClient() {
        Client client = null;
        try {
        //default settings must be "Settings.EMPTY"
        client = new PreBuiltTransportClient(Settings.EMPTY).addTransportAddress(
                         new TransportAddress(InetAddress.getByName(ES_HOSTNAME), ES_SVCNAME));
//      System.out.println(ES_HOSTNAME + ":" + ES_SVCNAME + " CONNECTED SUCCESSFUL!");
        } catch(Exception e) {
            e.printStackTrace();
        }
        return client;
    }
	
    /**
     * @param esIndexName
     */ 
    public static List<BSONObject> getAllRecordsFromES(String esIndexName) {
        List<BSONObject> objs = new ArrayList<>();
		
        Client client = createTransportClient();
        SearchResponse response = client.prepareSearch(esIndexName)
                                .setScroll(new TimeValue(60000))
                                .setSearchType(SearchType.DFS_QUERY_THEN_FETCH)
                                .setQuery(QueryBuilders.matchAllQuery())
                                .setSize(100)
                                .execute()
                                .actionGet();

        String[] clusterIds = new String[]{"_lid", "_cluid", "_cllid", "_idxlid"};
        for(SearchHit searchHit : response.getHits()) {
            Map<String, Object> sourceAsMap = searchHit.getSourceAsMap();
            Set<String> keySet = sourceAsMap.keySet();
            BSONObject obj = new BasicBSONObject();
            for(String string : keySet) {
                // remove keys about es cluster
                for(int i = 0; i < clusterIds.length; i++) {
                    if(string.contains(clusterIds[i])) {   break;   }
                    else if(i == clusterIds.length - 1) {
                        obj.put(string, sourceAsMap.get(string));
                        objs.add(obj);
                    }
                }			
            }
        }

        return objs;
    }
	
    /**
    * @param esIndexName
    */ 
    public static long getCountFromES(String esIndexName) {
        long count = 0;
        Client client = createTransportClient();
        SearchResponse response = client.prepareSearch(esIndexName)
                                  .setQuery(QueryBuilders.matchAllQuery())
                                  .setSearchType(SearchType.DFS_QUERY_THEN_FETCH)
                                  .setSize(0)
                                  .execute()
                                  .actionGet();
        count = response.getHits().totalHits;
        return count;
    }
	
    /**
     * @param esIndexName
     */ 
    public static int getCommitIDFromES(String esIndexName) {
        int commitID = -1;
		
        Client client = createTransportClient();
        SearchResponse response = client.prepareSearch(esIndexName)
                                  .setQuery(QueryBuilders.matchQuery("_id", "SDBCOMMIT"))
                                  .execute()
                                  .actionGet();
		
        for(SearchHit searchHit : response.getHits()) {
            Map<String, Object> sourceAsMap = searchHit.getSourceAsMap();
            commitID = (int)sourceAsMap.get("_lid");
        }
				
        return commitID;
    }

    /**
     * @param esIndexName
     */ 
    public static boolean isExistIndexInES(String esIndexName) {
        boolean isExist = false;
        int timeout = 300; // timeout 300s
        int doTimes = 0;
        int interval = 1; 
		
        Client client = createTransportClient();
        IndicesExistsResponse  existResponse = null;
        while(doTimes * interval < timeout) {	    	
            existResponse = client.admin().indices().exists(
                                  new IndicesExistsRequest().indices(esIndexName)).actionGet();
            isExist = existResponse.isExists();
			
            if(!isExist) {
                doTimes++;
                // interval 1s each time
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }else {
                break;		
            }
        }
	   		
        return isExist;
    }
	
}
