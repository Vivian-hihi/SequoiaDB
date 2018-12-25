package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * @Description: seqDB-16394 :: 指定不同格式分割符查询
 * @author fanyu
 * @Date:2018年11月26日
 * @version:1.0
 */

public class ListVersionsBySpecialDelimiter16394 extends S3TestBase {
    private boolean runSuccess1 = false;
    private boolean runSuccess2 = false;
    private boolean runSuccess3 = false;
    private boolean runSuccess4 = false;
    private String bucketName = "bucket16394";
    private String[] objectNames = { "!-_.*',()", "&@: $=+?",  "^`><{}[]#%\"~|" , "a1b2C3D4"/*,"\012atest!!!"*/,"abc"};
    private AmazonS3 s3Client = null;
    private int versionNum = 1;
    private File localPath = null;
    private String filePath = null;
    private int  fileSize = 10;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        filePath = localPath + File.separator + "localFile_" + fileSize  + ".txt";
        TestTools.LocalFile.createFile(filePath, fileSize );
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            System.out.println("objectName = " + objectName);//TODO:1、控制台正常输出信息建议删除
            for (int j = 0; j < versionNum; j++) {//TODO:2、只有一个版本没有必要for循环，可直接putObject
                s3Client.putObject(bucketName, objectName, new File(filePath));
            }
        }
    }
    //TODO:3、用例中有多个test，每个test请增加描述信息，如对应测试场景，另外这里多个test代码一致，建议用testng中的DtaProvider，只需要一份代码
    @Test
    private void testNormal() throws Exception {
        String delimiter = objectNames[3];
        VersionListing vsList = listVersions(bucketName,delimiter);
        List<String> expCommonPrefixes = new ArrayList<String>();
        expCommonPrefixes.add(objectNames[3]);
        List<String> expKeys = new ArrayList<String>();
        expKeys.add(objectNames[0]);
        expKeys.add(objectNames[1]);
        expKeys.add(objectNames[2]);
        expKeys.add(objectNames[4]);
        String[] versions = {"0", "0","0","0"};//TODO:4、用例中预置key只有一个版本，这里可以直接用初始版本0，下面多个test可以直接使用，不用每个test都定义该变量
        if(!vsList.isTruncated()) {
            checkResult(vsList, expCommonPrefixes, expKeys, versions);
        }else{
            Assert.fail("vsList.isTruncated() must be false");
        }
        runSuccess1 = true;
    }

    @Test
    private void testSpecial() throws Exception {
        String str = objectNames[0];
        String[] delimiters = str.split("");
        for(int i = 0; i < delimiters.length; i++) {
            VersionListing vsList = listVersions(bucketName, delimiters[i]);
            List<String> expCommonPrefixes = new ArrayList<String>();
            expCommonPrefixes.add(str.substring(0,i+1));
            List<String> expKeys = new ArrayList<String>();
            expKeys.add(objectNames[1]);
            expKeys.add(objectNames[2]);
            expKeys.add(objectNames[3]);
            expKeys.add(objectNames[4]);
            String[] versions = {"0", "0","0","0"};
            if(!vsList.isTruncated()) {
                checkResult(vsList, expCommonPrefixes, expKeys, versions);
            }else{
                Assert.fail("vsList.isTruncated() must be false");
            }
        }
        runSuccess2 = true;
    }

    @Test
    private void testSpecial1() throws Exception {
        String str = objectNames[1];
        String[] delimiters = str.split("");
        for(int i = 0; i < delimiters.length; i++) {
            VersionListing vsList = listVersions(bucketName, delimiters[i]);
            List<String> expCommonPrefixes = new ArrayList<String>();
            expCommonPrefixes.add(str.substring(0,i+1));
            List<String> expKeys = new ArrayList<String>();
            expKeys.add(objectNames[0]);
            expKeys.add(objectNames[2]);
            expKeys.add(objectNames[3]);
            expKeys.add(objectNames[4]);
            String[] versions = {"0","0","0","0"};
            if(!vsList.isTruncated()) {
                checkResult(vsList, expCommonPrefixes, expKeys, versions);
            }else{
                Assert.fail("vsList.isTruncated() must be false");
            }
        }
        runSuccess3 = true;
    }

    @Test
    private void testSpecial3() throws Exception {
        String str = objectNames[2];
        String[] delimiters = str.split("");
        for(int i = 0; i < delimiters.length; i++) {
            VersionListing vsList = listVersions(bucketName, delimiters[i]);
            List<String> expCommonPrefixes = new ArrayList<String>();
            expCommonPrefixes.add(str.substring(0,i+1));
            List<String> expKeys = new ArrayList<String>();
            expKeys.add(objectNames[0]);
            expKeys.add(objectNames[1]);
            expKeys.add(objectNames[3]);
            expKeys.add(objectNames[4]);
            String[] keys = {objectNames[0],objectNames[1],objectNames[3],objectNames[4]};//TODO:6、这里的keys没有使用，建议去掉
            String[] versions = {"0","0","0","0"};
            if(!vsList.isTruncated()) {
                checkResult(vsList, expCommonPrefixes, expKeys, versions);
            }else{
                Assert.fail("vsList.isTruncated() must be false");
            }
        }
        runSuccess4 = true;
    }
    //TODO:5、这里注释的代码请添加备注，如果是问题单请加上问题单号，如多余代码请删除
//    @Test
//    private void testSpecial5() throws Exception {
//        String str = objectNames[4];
//        String delimiter = "\010";
//        VersionListing vsList = listVersions(bucketName, delimiter);
//        List<String> expCommonPrefixes = new ArrayList<String>();
//        expCommonPrefixes.add("this is\010");
//        String[] keys = {objectNames[0], objectNames[1],objectNames[2],objectNames[3]};
//        String[] versions = {"0", "0", "0", "0"};
//        if (!vsList.isTruncated()) {
//            checkResult(vsList, expCommonPrefixes, keys, versions);
//        } else {
//            Assert.fail("vsList.isTruncated() must be false");
//        }
//        runSuccess4 = true;
//    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess1 && runSuccess2 && runSuccess3 && runSuccess4) {
                CommLib.clearBucket(s3Client, bucketName);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void checkResult(VersionListing vsList,List<String> commonPrefixes, List<String> expKeys, String[] expVersions) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size(),
                "actCommonPrefixes = " + actCommonPrefixes.toString()+",expCommonPrefixes = " + commonPrefixes.toString());
        Assert.assertEquals(actCommonPrefixes, commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes=" + commonPrefixes.toString());
       
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expVersions.length,"vsSummaryList = " + vsSummaryList.toString());
        String key = "";
        List<String> actKeys = new ArrayList<String>();
        for (int i = 0; i < vsSummaryList.size(); i++) {
            S3VersionSummary versionSummary = vsSummaryList.get(i);
            Assert.assertEquals(versionSummary.getBucketName(), bucketName);
            Assert.assertEquals(versionSummary.getVersionId(),expVersions[i]);
            if(!key.equals(versionSummary.getKey())){//TODO:4、这里的if判断没有意见，建议直接存获取的key名
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actkeys = " + actKeys + ",expkeys = " + objectNames);
    }

    private VersionListing listVersions(String bucketName, String delimiter) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setDelimiter(delimiter);
       // request.setEncodingType("url");
        return s3Client.listVersions(request);
    }
}
