package com.sequoias3.partupload;

import java.io.File;
import java.io.IOException;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * @Description seqDB-18683:上传分段号为降序
 * @Author wangkexin
 * @Date 2019.07.29
 */

public class UploadPart18683 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket18683";
    private String keyName = "key18683";
    private AmazonS3 s3Client = null;
    private long fileSize = 15 * 1024 * 1024;
    private long partSize = 3 * 1024 * 1024;
    private File localPath = null;
    private File file = null;
    private String filePath = null;
    private String uploadId = "";
    private List<PartETag> partEtags = new ArrayList<>();

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        file = new File(filePath);

        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(new CreateBucketRequest(bucketName));
    }

    @Test(enabled = false)
    private void testUpload() throws Exception {
        uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
        // upload part 1
        uploadPart(0, 1);
        // upload part 5
        uploadPart(partSize * 4, 5);
        // upload part 3
        uploadPart(partSize * 2, 3);
        // 完成分段上传
        try {
            // 由于客户端发送请求时会自动将partEtags中的内容按照partNumber升序排序，所以此处使用REST接口发送请求
            completeMultipartUpload();
            Assert.fail("the list of parts was not in ascending order should fail.");
        } catch (AmazonS3Exception e) {
            Assert.assertEquals(e.getErrorCode(), "InvalidPartOrder");
        }
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client, bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private void uploadPart(long filepositon, int partNumber) throws IOException {
        UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(filepositon)
                .withPartNumber(partNumber).withPartSize(partSize).withBucketName(bucketName).withKey(keyName)
                .withUploadId(uploadId);
        UploadPartResult uploadPartResult = s3Client.uploadPart(partRequest);
        partEtags.add(uploadPartResult.getPartETag());
        String expPartMd5 = TestTools.getFilePartMD5(file, filepositon, partSize);
        String actPartMd5 = uploadPartResult.getPartETag().getETag();
        Assert.assertEquals(actPartMd5, expPartMd5, "part number = " + uploadPartResult.getPartETag().getPartNumber());
    }

    private void completeMultipartUpload() throws Exception {
        HttpPost request = new HttpPost(S3TestBase.s3ClientUrl + "/" + URLEncoder.encode(bucketName, "UTF-8") + "/"
                + URLEncoder.encode(keyName, "UTF-8") + "?uploadId=" + uploadId);
        // RequestHeaders:
        request.setHeader("Authorization", UserCommDefind.authValPre + S3TestBase.s3AccessKeyId + "/");

        // Requeatbody:
        String completeMultipartUpload = "<CompleteMultipartUpload>";
        for (PartETag etag : partEtags) {
            completeMultipartUpload += "<Part><PartNumber>" + etag.getPartNumber() + "</PartNumber><ETag>"
                    + etag.getETag() + "</ETag></Part>";
        }
        completeMultipartUpload += "</CompleteMultipartUpload>";

        StringEntity testString = new StringEntity(completeMultipartUpload, StandardCharsets.UTF_8);
        request.setEntity(testString);
        CloseableHttpClient client = RestClient.createHttpClient();
        try {
            RestClient.sendRequest(client, request);
        } catch (Exception e) {// TODO 这里应该直接用实际抛出的异常类型
            throw httpToAmazon(e);
        }
    }

    // TODO 用公共方法RegionUtils.httpToAmazon
    private AmazonS3Exception httpToAmazon(Exception e) {
        AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
        amazonS3Exception.setErrorCode(getString(e.getMessage(), "Code"));
        amazonS3Exception.setErrorMessage(getString(e.getMessage(), "Message"));
        return amazonS3Exception;
    }

    // TODO 为什么还需要getString？同公共方法
    private String getString(String s, String flag) {
        int length = flag.length();
        String parttern = "<" + flag + ">.*</" + flag + ">";
        Pattern r = Pattern.compile(parttern);
        Matcher m = r.matcher(s);
        if (m.find()) {
            return m.group(0).substring(length + 2, m.group(0).length() - (length + 3));
        } else {
            return "NO MATCH";
        }
    }
}
