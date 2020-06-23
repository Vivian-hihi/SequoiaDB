package com.mongodb.utils;

import java.net.UnknownHostException;
import java.util.Arrays;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;
import org.springframework.data.mongodb.MongoDbFactory;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.convert.MongoConverter;
import org.springframework.data.mongodb.gridfs.GridFsTemplate;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.testng.AbstractTestNGSpringContextTests;
import org.testng.ITestContext;
import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;

import com.mongodb.DB;
import com.mongodb.MongoClient;
import com.mongodb.MongoClientOptions;
import com.mongodb.MongoCredential;
import com.mongodb.ServerAddress;

@ContextConfiguration(locations = { "classpath*:spring-mongodb-2x.xml" })
public class MongodbTestBase extends AbstractTestNGSpringContextTests {
    public static MongoClient mongoClient;
    public static String dbName;
    @Autowired
    public MongoDbFactory mongoDbFactory;
    @Autowired
    public MongoConverter converter;
    @Autowired
    public MongoTemplate mongoTemplate;
    @Autowired
    public GridFsTemplate gridFsTemplate;
    public Config config;

    public static MongoClient getClient( String hostname, int port,
            String username, String password ) throws UnknownHostException {
        MongoClientOptions opt = MongoClientOptions.builder().build();
        MongoCredential credential = MongoCredential.createMongoCRCredential(
                username, dbName, password.toCharArray() );
        return new MongoClient( new ServerAddress( hostname, port ),
                Arrays.asList( credential ), opt );
    }

    /**
     * @Description: 获取mongodb客户端
     * @param hostname
     *            主机名
     * @param port
     *            端口号
     * @return
     * @throws UnknownHostException
     */
    public static MongoClient getClient( String hostname, int port )
            throws UnknownHostException {
        MongoClientOptions opt = MongoClientOptions.builder().build();
        return new MongoClient( new ServerAddress( hostname, port ), opt );
    }

    /**
     * @Description: 获取mongodb客户端
     * @return
     * @throws UnknownHostException
     */
    public static MongoClient getClient() throws UnknownHostException {
        return mongoClient;
    }

    /**
     * @Description: 获取默认的mongodb数据库
     * @return
     * @throws UnknownHostException
     */
    public static DB getDB( MongoClient client ) {
        return client.getDB( dbName );
    }

    public static void dropCL( DB db, String... clNames ) {
        for ( String clName : clNames ) {
            if ( db.getCollectionNames().contains( clName ) ) {
                db.getCollection( clName ).drop();
            }
        }
    }

    public static void dropCL( MongoTemplate mongoTemplate,
            String... clNames ) {
        for ( String clName : clNames ) {
            if ( mongoTemplate.collectionExists( clName ) ) {
                mongoTemplate.dropCollection( clName );
            }
        }
    }

    /**
     * @Description 根据每个用例测试结果决定是否删除cl。用例失败则不删除；用例成功则删除
     * @param context
     *            testng上下文
     * @param classString
     *            类的toString，如：this.toString()
     * @param db
     *            数据库实例
     * @param clNames
     *            集合
     */
    public static void dropCLByTestResult( ITestContext context,
            String classString, DB db, String... clNames ) {
        if ( !context.getFailedTests().getAllResults().toString()
                .contains( classString )
                && !context.getSkippedTests().toString()
                        .contains( classString ) ) {
            dropCL( db, clNames );
        }
    }

    /**
     * @Description 根据每个用例测试结果删除cl。用例失败则不删除；用例成功则删除
     * @param context
     *            testng上下文
     * @param classString
     *            类的toString，如：this.toString()
     * @param clNames
     *            集合名
     */
    public static void dropCLByTestResult( ITestContext context,
            String classString, MongoTemplate mongoTemplate,
            String... clNames ) {
        if ( !context.getFailedTests().getAllResults().toString()
                .contains( classString )
                && !context.getSkippedTests().toString()
                        .contains( classString ) ) {
            dropCL( mongoTemplate, clNames );
        }
    }

    /**
     * @Description: 初始化测试套
     * @throws UnknownHostException
     */
    @BeforeSuite(alwaysRun = true)
    public void initSuite() throws UnknownHostException {
        logger.info( "begin init suite..." );
        ApplicationContext ctx = new ClassPathXmlApplicationContext(
                "spring-mongodb-2x.xml" );
        config = ( Config ) ctx.getBean( "config" );
        logger.info( "config:" + config );
        mongoClient = getClient( config.getHost(), config.getPort() );
        dbName = config.getDbName();
        cleanEnv();
        logger.info( "end init suite..." );
    }

    /**
     * @Description 结束测试套，如果有用例失败，不会清理环境； 如果用例全部执行成功，会清理环境
     * @param context
     *            testng上下文
     * @throws Exception
     */
    @AfterSuite(alwaysRun = true)
    public void finiSuite( ITestContext context ) throws Exception {
        logger.info( "begin finish Suite..." );
        try {
            if ( context.getFailedTests().size() == 0 ) {
                cleanEnv();
            }
        } finally {
            mongoClient.close();
        }
        logger.info( "end finish Suite..." );
    }

    /**
     * @Description: 清理环境
     * @throws UnknownHostException
     */
    public void cleanEnv() throws UnknownHostException {
        try {
            DB db = mongoClient.getDB( config.getDbName() );
            db.dropDatabase();
        } catch ( Exception e ) {
            e.printStackTrace();
            logger.error( e.getMessage() );
        }

        try {
            DB db = mongoClient.getDB( config.getDbname1() );
            db.dropDatabase();
        } catch ( Exception e ) {
            e.printStackTrace();
            logger.error( e.getMessage() );
        }
    }
}
