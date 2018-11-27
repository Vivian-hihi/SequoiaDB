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

    @SuppressWarnings("resource")
    public static Client createTransportClient(String esHostName, int port) {
        System.setProperty("es.set.netty.runtime.available.processors", "false");
        Client client = null;
        try {
        //default settings must be "Settings.EMPTY"
        client = new PreBuiltTransportClient(Settings.EMPTY).addTransportAddress(
                         new TransportAddress(InetAddress.getByName(esHostName), port));
        } catch(Exception e) {
            e.printStackTrace();
        }
        return client;
    }
	
    /**
     * @param esClient
     * @param esIndexName
     */ 
    public static List<BSONObject> getAllRecordsFromES(Client esClient, String esIndexName) {
        List<BSONObject> objs = new ArrayList<>();
		
        SearchResponse response = esClient.prepareSearch(esIndexName)
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
    * @param esClient
    * @param esIndexName
    */ 
    public static long getCountFromES(Client esClient, String esIndexName) {
        long count = 0;
        SearchResponse response = esClient.prepareSearch(esIndexName)
                                  .setQuery(QueryBuilders.matchAllQuery())
                                  .setSearchType(SearchType.DFS_QUERY_THEN_FETCH)
                                  .setSize(0)
                                  .execute()
                                  .actionGet();
        count = response.getHits().totalHits;
        return count;
    }
	
    /**
     * @param esClient    
     * @param esIndexName
     */ 
    public static int getCommitIDFromES(Client esClient, String esIndexName) {
        int commitID = -1;
		
        SearchResponse response = esClient.prepareSearch(esIndexName)
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
     * @param esClient
     * @param esIndexName
     */ 
    public static boolean isExistIndexInES(Client esClient, String esIndexName) {
        boolean isExist = false;
        int timeout = 300; // timeout 300s
        int doTimes = 0;
        int interval = 1; 
		
        IndicesExistsResponse  existResponse = null;
        while(doTimes * interval < timeout) {	    	
            existResponse = esClient.admin().indices().exists(
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

   /**
    * @param esClient   
    * @param esIndexName
    */ 
    public static boolean isIndexDeletedInES(Client esClient, String esIndexName) {
        boolean isDeleted = true;
        int timeout = 300; // timeout 300s
        int doTimes = 0;
        int interval = 1;
 
        IndicesExistsResponse  existResponse = null;
        while(doTimes * interval < timeout) {	    	
            existResponse = esClient.admin().indices().exists(
                   new IndicesExistsRequest().indices(esIndexName)).actionGet();
            isDeleted = !existResponse.isExists();
            
            if(!isDeleted) {
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
   		
        return isDeleted;
    }
     
}
