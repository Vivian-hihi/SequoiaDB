package com.sequoiadb.utils;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.elasticsearch.action.ActionFuture;
import org.elasticsearch.action.admin.indices.delete.DeleteIndexRequest;
import org.elasticsearch.action.admin.indices.delete.DeleteIndexResponse;
import org.elasticsearch.action.admin.indices.exists.indices.IndicesExistsRequest;
import org.elasticsearch.action.admin.indices.exists.indices.IndicesExistsResponse;
import org.elasticsearch.action.admin.indices.stats.IndicesStatsRequest;
import org.elasticsearch.action.admin.indices.stats.IndicesStatsResponse;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.action.search.SearchType;
import org.elasticsearch.client.Client;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.index.query.QueryBuilders;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.transport.client.PreBuiltTransportClient;

/**
 * ES的公共类，涉及ES内部操作的方法放于此类
 */
public class FullTextESUtils {

    /**
     * 获取elasticsearch的连接
     * 
     * @param esHostName
     * @param port
     * @return Client 如果建连成功则返回连接，否则返回NULL
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    @SuppressWarnings("resource")
    public static Client createTransportClient( String esHostName, int port ) {
        System.setProperty( "es.set.netty.runtime.available.processors", "false" );
        Client client = null;
        try {
            // 默认设置一定要设成 "Settings.EMPTY"
            client = new PreBuiltTransportClient( Settings.EMPTY )
                    .addTransportAddress( new TransportAddress( InetAddress.getByName( esHostName ), port ) );
        } catch ( Exception e ) {
            e.printStackTrace();
        }
        return client;
    }

    /**
     * 获取elasticsearch端的所有全文索引记录
     * 
     * @param esClient
     * @param esIndexName
     * @return List< BSONObject > 返回ES端的全文索引记录
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<BSONObject> getAllRecordsFromES( Client esClient, String esIndexName ) {
        List<BSONObject> objs = new ArrayList<>();

        SearchResponse response = esClient.prepareSearch( esIndexName ).setScroll( new TimeValue( 60000 ) )
                .setSearchType( SearchType.DFS_QUERY_THEN_FETCH ).setQuery( QueryBuilders.matchAllQuery() )
                .setSize( 100 ).execute().actionGet();

        String[] clusterIds = new String[] { "_lid", "_cluid", "_cllid", "_idxlid" };
        for ( SearchHit searchHit : response.getHits() ) {
            Map<String, Object> sourceAsMap = searchHit.getSourceAsMap();
            Set<String> keySet = sourceAsMap.keySet();
            BSONObject obj = new BasicBSONObject();
            for ( String string : keySet ) {
                // 去掉SDBCOMMITID这条记录
                boolean isSDBCOMMITID = false;
                for ( String item:  clusterIds) {
                    if ( string.contains( item ) ) {
                        isSDBCOMMITID = true;
                        break;
                    } 
                }
                
                if( !isSDBCOMMITID ) {
                    obj.put( string, sourceAsMap.get( string ) );
                    objs.add( obj );
                }
               
            }
        }

        return objs;
    }

    /**
     * 获取elasticsearch端全文索引的总记录数
     * 
     * @param esClient
     * @param esIndexName
     * @return long 返回记录总数
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static long getCountFromES( Client esClient, String esIndexName ) {
        SearchResponse response = esClient.prepareSearch( esIndexName ).setQuery( QueryBuilders.matchAllQuery() )
                .setSearchType( SearchType.DFS_QUERY_THEN_FETCH ).setSize( 0 ).execute().actionGet();
        return response.getHits().totalHits;
    }

    /**
     * 获取elasticsearch端的SDBCOMMIT记录下的逻辑ID值
     * 
     * @param esClient
     * @param esIndexName
     * @return int 返回SDBCOMMIT._lid值
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static int getCommitIDFromES( Client esClient, String esIndexName ) {
        int commitID = -1;

        SearchResponse response = esClient.prepareSearch( esIndexName )
                .setQuery( QueryBuilders.matchQuery( "_id", "SDBCOMMIT" ) )
                .execute().actionGet();

        for ( SearchHit searchHit : response.getHits() ) {
            Map<String, Object> sourceAsMap = searchHit.getSourceAsMap();
            commitID = (int) sourceAsMap.get( "_lid" );
        }

        return commitID;
    }

    /**
     * 获取elasticsearch端的SDBCOMMIT记录下的原始集合逻辑ID值
     * 
     * @param esClient
     * @param esIndexName
     * @return int 返回SDBCOMMIT._cllid值
     * @Author liuxiaoxuan
     * @Date 2019-05-16
     */
    public static int getCommitCLLIDFromES( Client esClient,
            String esIndexName ) {
        int commitCLLID = -1;

        SearchResponse response = esClient.prepareSearch( esIndexName )
                .setQuery( QueryBuilders.matchQuery( "_id", "SDBCOMMIT" ) )
                .execute().actionGet();

        for ( SearchHit searchHit : response.getHits() ) {
            Map<String, Object> sourceAsMap = searchHit.getSourceAsMap();
            commitCLLID = (int) sourceAsMap.get( "_cllid" );
        }

        return commitCLLID;
    }
    
    /**
     * 获取多个elasticsearch端的SDBCOMMIT记录下的原始集合逻辑ID值
     * 
     * @param esClient
     * @param esIndexNames
     * @return List < Integer > 返回每个全文索引的SDBCOMMIT._cllid值
     * @Author liuxiaoxuan
     * @Date 2019-05-17
     */
    public static List < Integer > getCommitCLLIDFromES( Client esClient,
            List < String > esIndexNames ) {
        List < Integer > commitCLLIDs = new ArrayList<>();

        for ( String esIndexName : esIndexNames ) {
            commitCLLIDs.add( getCommitCLLIDFromES( esClient, esIndexName ) );
        }
         
        return commitCLLIDs;
    }

    /**
     * 判断elasticsearch端的全文索引名是否存在
     * 
     * @param esClient
     * @param esIndexName
     * @param expExist 预期该索引是否存在
     * @return boolean 如果预期存在则返回true，预期不存在则返回false
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static boolean isExistIndexInES( Client esClient, String esIndexName, boolean expExist ) {
        int timeout = 600; // 超时时间600s
        int doTimes = 0;
        int interval = 1;

        IndicesExistsResponse existResponse = null;
        while ( doTimes * interval < timeout ) {
            existResponse = esClient.admin().indices().exists( new IndicesExistsRequest().indices( esIndexName ) )
                    .actionGet();

            if ( expExist != existResponse.isExists() ) {
                doTimes++;
                // 每次循环间隔1s
                try {
                    Thread.sleep( interval * 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            } else {
                // 防止读到旧数据,由于es端每隔5s刷新一次,因此这里需要等待6s
                try {
                    Thread.sleep( 6000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
                break;
            }
        }

        if( doTimes * interval == timeout ) {  System.out.println( esIndexName + " is mapping to ES timeout" );  }
        return existResponse.isExists();
    }

    /**
     * 判断elasticsearch端同一集合上对应的多个全文索引名是否存在
     * 
     * @param esClient
     * @param esIndexNames
     * @param expExist  预期该索引是否存在
     * @return boolean 存在返回true, 否则返回false
     * @Author liuxiaoxuan 如果预期存在则返回true，预期不存在则返回false
     * @Date 2018-11-15
     */
    public static boolean isExistIndexInES( Client esClient, List < String > esIndexNames, boolean expExist ) {
        for ( String esIndexName : esIndexNames ) {            
            return isExistIndexInES( esClient, esIndexName, expExist );
        }
        
        System.out.println("esIndexName is null");
        return false;        
    }
    
    /**
     * 判断elasticsearch端的所有全文索引数据是否已被删除
     * 
     * @param esClient
     * @return boolean
     * @Author liuxiaoxuan
     * @Date 2018-12-20
     */
    public static boolean isDeleteAllIndices( Client esClient ) {
        boolean isSuccess = false;
        DeleteIndexRequest deleteRequest = new DeleteIndexRequest( "_all" );
        DeleteIndexResponse deleteResponse = esClient.admin().indices().delete( deleteRequest ).actionGet();
        isSuccess = deleteResponse.isAcknowledged();
        return isSuccess;
    }

    /**
     * 打印elasticsearch端的所有全文索引名
     * 
     * @param esClient
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-12-20
     */
    public static void printAllIndices( Client esClient ) {
        ActionFuture<IndicesStatsResponse> isr = esClient.admin().indices().stats( new IndicesStatsRequest().all() );
        Set<String> sets = isr.actionGet().getIndices().keySet();
        System.out.println( "get all indieces in ES: " );
        for ( String set : sets ) {
            System.out.println( set );
        }
    }

    /**
     * 清理elasticsearch端的所有全文索引
     * 
     * @param esClient
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-12-20
     */
    public static void clearAllIndicesInES( Client esClient ) {
        if ( !FullTextESUtils.isDeleteAllIndices( esClient ) ) {
            FullTextESUtils.printAllIndices( esClient );
        }
    }
}
