
package com.mongodb.java.serial;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.bson.Document;
import org.bson.conversions.Bson;
import org.testng.Assert;
import org.testng.ITestContext;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.mongodb.MongoClient;
import com.mongodb.MongoClientOptions;
import com.mongodb.MongoCommandException;
import com.mongodb.MongoCredential;
import com.mongodb.ServerAddress;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.model.Filters;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.utils.MongodbTestBase;

/**
 * @Description seqDB-21934:增删查用户
 * @author chimanzhao
 * @version 1.00
 * @Date 2020/6/23
 */

public class Authentication21934 extends MongodbTestBase {
    private MongoClientOptions opt = null;
    private MongoClient client1 = null;
    private MongoDatabase db1 = null;
    private String username1 = "21934A";
    private String username2 = "21934B";
    private String dbName = "db21934A";
    private String clName = "cl21934";
    private int num = 30;
    private List< Document > list;

    @BeforeClass
    public void setup() {
        opt = MongoClientOptions.builder().build();
        client1 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ), opt );
        client1.dropDatabase( dbName );
        db1 = client1.getDatabase( dbName );
        list = new ArrayList<>();
        for ( int i = 0; i < num; i++ ) {
            list.add( new Document( "a", i ).append( "b", "" + i )
                    .append( "c", Arrays.asList( i + 1, i + 2, i + 3 ) )
                    .append( "d", "test-" + i + "-" + i % 3 )
                    .append( "e", new Document( "a", 1 ).append( "b", 1 ) ) );
        }
    }

    @Test
    @SuppressWarnings({ "unchecked", "resource" })
    public void Test() {
        // 创建多个用户
        db1.runCommand( new Document( "createUser", username1 )
                .append( "pwd", username1 )
                .append( "roles", Collections.singletonList( "dbOwner" ) ) );
        // 重复创建用户
        try {
            db1.runCommand( new Document( "createUser", username2 )
                    .append( "pwd", username2 ).append( "roles",
                            Collections.singletonList( "dbOwner" ) ) );

            db1.runCommand( new Document( "createUser", username2 )
                    .append( "pwd", username2 ).append( "roles",
                            Collections.singletonList( "dbOwner" ) ) );
        } catch ( MongoCommandException e ) {
            if ( e.getErrorCode() != -295 ) {
                throw e;
            }
        }

        // 查询用户
        // 查询所有用户
        Document userInfos = db1.runCommand( new Document( "usersInfo", 1 ) );
        List< Document > users = userInfos.get( "users", ArrayList.class );
        Assert.assertEquals( users,
                Arrays.asList( new Document( "user", username1 ),
                        new Document( "user", username2 ) ) );
        System.out.println( "users = " + users.toString() );
        System.out.println( "userInfos = " + userInfos.toString() );

        // 查询单个用户
        Document userInfo = db1.runCommand( new Document( "usersInfo",
                Collections.singletonList( new Document( "user", username1 )
                        .append( "db", dbName ) ) ) );
        System.out.println( "userInfo = " + userInfo.toString() );
        // TODO:SEQUOIADBMAINSTREAM-5974
        List< Document > user = userInfos.get( "users", ArrayList.class );
        System.out.print( user );
        // Assert.assertEquals( user,
        // Collections.singletonList( new Document( "user", username1 ) ) );

        // 创建连接
        MongoCredential mongoCredential = MongoCredential
                .createScramSha1Credential( username1, dbName,
                        username1.toCharArray() );
        MongoClient client2 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredential ), opt );

        MongoCredential mongoCredentia2 = MongoCredential
                .createScramSha1Credential( username2, dbName,
                        username2.toCharArray() );
        MongoClient client3 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia2 ), opt );

        MongoDatabase db1 = client1.getDatabase( dbName );
        // 创建集合
        db1.createCollection( clName );

        MongoDatabase db2 = client2.getDatabase( dbName );
        MongoCollection< Document > cl2 = db2.getCollection( clName );
        // 增加/查询记录
        cl2.insertMany( list );
        List< Document > actList = cl2.find()
                .into( new ArrayList< Document >() );
        Assert.assertEquals( actList, list );

        MongoDatabase db3 = client3.getDatabase( dbName );
        MongoCollection< Document > cl3 = db3.getCollection( clName );
        // 删除记录
        Bson query;
        DeleteResult result;
        // 删除记录
        query = Filters.eq( "a", 0 );
        Assert.assertEquals( cl3.count( query ), 1 );
        result = cl3.deleteOne( query );
        Assert.assertEquals( result.getDeletedCount(), 1 );
        Assert.assertEquals( cl3.count( query ), 0 );

        // 删除用户
        db2.runCommand( new Document( "dropUser", username1 ) );
        db3.runCommand( new Document( "dropUser", username2 ) );
        // 重复删除用户
        try {
            db2.runCommand( new Document( "dropUser", username1 ) );
            Assert.fail( "exp fail but success!" );
        } catch ( MongoCommandException e ) {
            if ( e.getErrorCode() != -300 ) {
                throw e;
            }
        }

        // 查询所有用户
        Document userInfos1 = db2.runCommand( new Document( "usersInfo", 1 ) );
        System.out.println( "userInfos = " + userInfos1.toString() );

        // 删除集合
        cl3.drop();

        // 使用不存在的用户创建连接
        MongoCredential mongoCredentia3 = MongoCredential
                .createScramSha1Credential( "noexists", dbName,
                        "noexists".toCharArray() );
        MongoClient client4 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia3 ), opt );
        MongoDatabase db4 = client4.getDatabase( dbName );
        MongoCollection< Document > cl4 = db4.getCollection( clName );
        try {
            cl4.insertOne( new Document( "a", 1 ) );
            Assert.fail( "exp fail but act success!!!" );
        } catch ( Exception e ) {
            if ( !e.getMessage().contains( "Exception authenticating" ) ) {
                throw e;
            }
        }
        db1.drop();

        client2.close();
        client3.close();
    }

    @AfterClass
    public void tearDown( ITestContext context ) {
    }

}
