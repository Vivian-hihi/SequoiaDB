package com.sequoiadb.utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.net.InetAddress;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.elasticsearch.action.ActionFuture;
import org.elasticsearch.action.admin.indices.delete.DeleteIndexRequest;
import org.elasticsearch.action.admin.indices.delete.DeleteIndexResponse;
import org.elasticsearch.action.admin.indices.exists.indices.IndicesExistsRequest;
import org.elasticsearch.action.admin.indices.exists.indices.IndicesExistsResponse;
import org.elasticsearch.action.admin.indices.stats.IndexStats;
import org.elasticsearch.action.admin.indices.stats.IndicesStatsRequest;
import org.elasticsearch.action.admin.indices.stats.IndicesStatsResponse;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.action.search.SearchType;
import org.elasticsearch.client.*;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.transport.client.PreBuiltTransportClient;
import org.elasticsearch.index.query.*;
import org.elasticsearch.search.SearchHit;

/**
 * ES的公共类，涉及ES内部操作的方法放于此类
 */
public class FullTextESUtils {

    /**
     * 获取elasticsearch的连接
     * @param esHostName
     * @param port
     * @return Client 返回连接
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    @SuppressWarnings("resource")
    public static Client createTransportClient( String esHostName, int port ) {
        System.setProperty( "es.set.netty.runtime.available.processors",
                "false" );
        Client client = null;
        try {
            // default settings must be "Settings.EMPTY"
            client = new PreBuiltTransportClient( Settings.EMPTY )
                    .addTransportAddress( new TransportAddress(
                            InetAddress.getByName( esHostName ), port ) );
        } catch ( Exception e ) {
            e.printStackTrace();
        }
        return client;
    }

    /**
     * 获取elasticsearch端的所有全文索引记录
     * @param esClient
     * @param esIndexName
     * @return List< BSONObject > 返回记录
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List< BSONObject > getAllRecordsFromES( Client esClient,
            String esIndexName ) {
        List< BSONObject > objs = new ArrayList<>();

        SearchResponse response = esClient.prepareSearch( esIndexName )
                .setScroll( new TimeValue( 60000 ) )
                .setSearchType( SearchType.DFS_QUERY_THEN_FETCH )
                .setQuery( QueryBuilders.matchAllQuery() ).setSize( 100 )
                .execute().actionGet();

        String[] clusterIds = new String[] { "_lid", "_cluid", "_cllid",
                "_idxlid" };
        for ( SearchHit searchHit : response.getHits() ) {
            Map< String, Object > sourceAsMap = searchHit.getSourceAsMap();
            Set< String > keySet = sourceAsMap.keySet();
            BSONObject obj = new BasicBSONObject();
            for ( String string : keySet ) {
                // remove keys about es cluster
                for ( int i = 0; i < clusterIds.length; i++ ) {
                    if ( string.contains( clusterIds[ i ] ) ) {
                        break;
                    } else if ( i == clusterIds.length - 1 ) {
                        obj.put( string, sourceAsMap.get( string ) );
                        objs.add( obj );
                    }
                }
            }
        }

        return objs;
    }

    /**
     * 获取elasticsearch端全文索引的总记录数
     * @param esClient
     * @param esIndexName
     * @return long 返回记录总数
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static long getCountFromES( Client esClient, String esIndexName ) {
        long count = 0;
        SearchResponse response = esClient.prepareSearch( esIndexName )
                .setQuery( QueryBuilders.matchAllQuery() )
                .setSearchType( SearchType.DFS_QUERY_THEN_FETCH ).setSize( 0 )
                .execute().actionGet();
        count = response.getHits().totalHits;
        return count;
    }

    /**
     * 获取elasticsearch端的SDBCOMMITID值
     * @param esClient
     * @param esIndexName
     * @return int 返回SDBCOMMITID值
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static int getCommitIDFromES( Client esClient, String esIndexName ) {
        int commitID = -1;

        SearchResponse response = esClient.prepareSearch( esIndexName )
                .setQuery( QueryBuilders.matchQuery( "_id", "SDBCOMMIT" ) )
                .execute().actionGet();

        for ( SearchHit searchHit : response.getHits() ) {
            Map< String, Object > sourceAsMap = searchHit.getSourceAsMap();
            commitID = ( int ) sourceAsMap.get( "_lid" );
        }

        return commitID;
    }

    /**
     * 判断elasticsearch端的全文索引名是否存在，用于检查在创建阶段索引名是否映射到elasticsearch端
     * @param esClient
     * @param esIndexName
     * @return boolean 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static boolean isExistIndexInES( Client esClient,
            String esIndexName ) {
        boolean isExist = false;
        int timeout = 600; // timeout 600s
        int doTimes = 0;
        int interval = 1;

        IndicesExistsResponse existResponse = null;
        while ( doTimes * interval < timeout ) {
            existResponse = esClient.admin().indices()
                    .exists( new IndicesExistsRequest().indices( esIndexName ) )
                    .actionGet();
            isExist = existResponse.isExists();

            if ( !isExist ) {
                doTimes++;
                // interval 1s each time
                try {
                    Thread.sleep( 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            } else {
                // sleep for 6s, in case of old fulltext is read, because es
                // will refresh interval 5s
                try {
                    Thread.sleep( 6000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
                break;
            }
        }

        return isExist;
    }

    /**
     * 判断elasticsearch端的全文索引名是否被删除，用于清理阶段作环境检查
     * @param esClient
     * @param esIndexName
     * @return boolean 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static boolean isIndexDeletedInES( Client esClient,
            String esIndexName ) {
        boolean isDeleted = true;
        int timeout = 600; // timeout 600s
        int doTimes = 0;
        int interval = 1;

        IndicesExistsResponse existResponse = null;
        while ( doTimes * interval < timeout ) {
            existResponse = esClient.admin().indices()
                    .exists( new IndicesExistsRequest().indices( esIndexName ) )
                    .actionGet();
            isDeleted = !existResponse.isExists();

            if ( !isDeleted ) {
                doTimes++;
                // interval 1s each time
                try {
                    Thread.sleep( 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            } else {
                break;
            }
        }

        return isDeleted;
    }

    /**
     * 判断elasticsearch端的所有全文索引数据是否已被删除
     * @param esClient
     * @return boolean 
     * @Author liuxiaoxuan
     * @Date 2018-12-20
     */
    public static boolean isDeleteAllIndices( Client esClient ) {
        boolean isSuccess = false;
        DeleteIndexRequest deleteRequest = new DeleteIndexRequest( "_all" );
        DeleteIndexResponse deleteResponse = esClient.admin().indices()
                .delete( deleteRequest ).actionGet();
        isSuccess = deleteResponse.isAcknowledged();
        return isSuccess;
    }

    /**
     * 打印elasticsearch端的所有全文索引名
     * @param esClient
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-12-20
     */
    public static void printAllIndices( Client esClient ) {
        ActionFuture< IndicesStatsResponse > isr = esClient.admin().indices()
                .stats( new IndicesStatsRequest().all() );
        Set< String > sets = isr.actionGet().getIndices().keySet();
        System.out.println( "get all indieces in ES: " );
        for ( String set : sets ) {
            System.out.println( set );
        }
    }

    /**
     * 清理elasticsearch端的所有全文索引
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
