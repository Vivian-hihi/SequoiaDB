
package com.mongodb.springdata.serial;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.springframework.data.mongodb.UncategorizedMongoDbException;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.testng.Assert;
import org.testng.ITestContext;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.mongodb.BasicDBObject;
import com.mongodb.CommandResult;
import com.mongodb.MongoClient;
import com.mongodb.MongoClientOptions;
import com.mongodb.MongoCommandException;
import com.mongodb.MongoCredential;
import com.mongodb.ServerAddress;
import com.mongodb.utils.Entity;
import com.mongodb.utils.MongodbTestBase;

/**
 * @Description seqDB-21934:增删查用户
 * @author chimanzhao
 * @version 1.00
 * @Date 2020/6/23
 */

public class Authentication21934 extends MongodbTestBase {
    private MongoTemplate mongoTemplate;
    private MongoClient client1;
    private MongoClientOptions opt = null;
    private String username1 = "spring21934A";
    private String username2 = "spring21934B";
    private String dbName = "spring_db21934B";
    private String clName = "cl21934";
    private int num = 6;
    private List< Entity > list;

    @BeforeClass
    public void setup() throws UnknownHostException {
        list = new ArrayList<>();
        for ( int i = 0; i < num; i++ ) {
            list.add(
                    new Entity( "a" + i, Entity.SEXS[ i % Entity.SEXS.length ],
                            i, i, Entity.COURSES ) );
        }
    }

    @Test
    @SuppressWarnings("unchecked")
    public void Test() throws UnknownHostException {
        opt = MongoClientOptions.builder().build();
        client1 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ), opt );
        mongoTemplate = new MongoTemplate( client1, dbName );
        mongoTemplate.getDb().dropDatabase();
        // 创建多个用户
        mongoTemplate
                .executeCommand( new BasicDBObject( "createUser", username1 )
                        .append( "pwd", username1 ).append( "roles",
                                Collections.singletonList( "dbOwner" ) ) );
        // 重复创建用户
        try {
            mongoTemplate.executeCommand(
                    new BasicDBObject( "createUser", username2 )
                            .append( "pwd", username2 ).append( "roles",
                                    Collections.singletonList( "dbOwner" ) ) );

            mongoTemplate.executeCommand(
                    new BasicDBObject( "createUser", username2 )
                            .append( "pwd", username2 ).append( "roles",
                                    Collections.singletonList( "dbOwner" ) ) );
        } catch ( MongoCommandException e ) {
            if ( e.getErrorCode() != -295 ) {
                throw e;
            }
        }

        // 查询用户
        // 查询所有用户
        CommandResult userInfos = mongoTemplate
                .executeCommand( new BasicDBObject( "usersInfo", 1 ) );
        System.out.println( "userInfos = " + userInfos.toString() );
        List< BasicDBObject > users = ( List< BasicDBObject > ) userInfos
                .get( "users" );
        Assert.assertEquals( users,
                Arrays.asList( new BasicDBObject( "user", username1 ),
                        new BasicDBObject( "user", username2 ) ) );

        // 查询单个用户
        CommandResult userInfo = mongoTemplate
                .executeCommand( new BasicDBObject( "usersInfo",
                        Collections.singletonList(
                                new BasicDBObject( "user", username1 )
                                        .append( "db", dbName ) ) ) );
        // TODO:SEQUOIADBMAINSTREAM-5974
        List< BasicDBObject > user = ( List< BasicDBObject > ) userInfo
                .get( "users" );
        // Assert.assertEquals( user,
        // Collections.singletonList( new Document( "user", username1 ) ) );

        // 创建连接
        MongoCredential mongoCredential = MongoCredential
                .createScramSha1Credential( username1, dbName,
                        username1.toCharArray() );
        MongoClient client2 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredential ), opt );
        MongoTemplate mongoTemplate2 = new MongoTemplate( client2, dbName );

        MongoCredential mongoCredentia2 = MongoCredential
                .createScramSha1Credential( username2, dbName,
                        username2.toCharArray() );
        MongoClient client3 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia2 ), opt );
        MongoTemplate mongoTemplate3 = new MongoTemplate( client3, dbName );

        // 创建集合
        mongoTemplate.createCollection( clName );

        // 增加/查询记录
        mongoTemplate2.insert( list, clName );
        List< Entity > actList = mongoTemplate2.findAll( Entity.class, clName );
        Assert.assertEquals( actList, list );

        // 删除记录
        Criteria criteria = Criteria.where( "name" ).is( "a0" );
        Query query = new Query( criteria );
        // 删除记录
        Assert.assertEquals( mongoTemplate3.count( query, clName ), 1 );
        mongoTemplate3.remove( query, clName );
        Assert.assertEquals( mongoTemplate3.count( query, clName ), 0 );

        // 使用不存在的用户创建连接
        MongoCredential mongoCredentia4 = MongoCredential
                .createScramSha1Credential( "noexists", dbName,
                        "noexists".toCharArray() );
        MongoClient client5 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia4 ), opt );
        MongoTemplate mongoTemplate5 = new MongoTemplate( client5, dbName );
        try {
            mongoTemplate5.insert(
                    new Entity( "a", Entity.SEXS[ 0 % Entity.SEXS.length ], 1,
                            1, Entity.COURSES ) );
        } catch ( UncategorizedMongoDbException e ) {
            if ( !e.getMessage().contains( "Authentication failed" ) ) {
                throw e;
            }
        }

        // 删除用户
        mongoTemplate2
                .executeCommand( new BasicDBObject( "dropUser", username1 ) );
        mongoTemplate3
                .executeCommand( new BasicDBObject( "dropUser", username2 ) );
        // 重复删除用户
        mongoTemplate2
                .executeCommand( new BasicDBObject( "dropUser", username1 ) );

        // 查询所有用户
        CommandResult userInfos1 = mongoTemplate
                .executeCommand( new BasicDBObject( "usersInfo", 1 ) );
        System.out.println( "userInfos = " + userInfos1.toString() );

        // 删除集合
        mongoTemplate2.dropCollection( clName );

        // 使用不存在的用户创建连接
        MongoCredential mongoCredentia3 = MongoCredential
                .createScramSha1Credential( "noexists", dbName,
                        "noexists".toCharArray() );
        MongoClient client4 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia3 ), opt );
        MongoTemplate mongoTemplate4 = new MongoTemplate( client4, dbName );
        try {
            mongoTemplate4.insert(
                    new Entity( "a", Entity.SEXS[ 0 % Entity.SEXS.length ], 1,
                            1, Entity.COURSES ) );
        } catch ( UncategorizedMongoDbException e ) {
            if ( !e.getMessage()
                    .contains( "Authentication has been disabled," ) ) {
                throw e;
            }
        }
        mongoTemplate.getDb().dropDatabase();
        client1.close();
        client2.close();
        client3.close();
        client4.close();
        client5.close();
    }

    @AfterClass
    public void tearDown( ITestContext context ) {
    }

}
