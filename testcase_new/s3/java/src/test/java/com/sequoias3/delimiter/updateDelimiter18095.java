package com.sequoias3.delimiter;

import java.io.IOException;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;

/**
 * @Description seqDB-18095: get delimiter,specified bucket does not exist.
 * @author wuyan
 * @Date 2019.04.10
 * @version 1.00
 */
public class updateDelimiter18095 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18095";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws IOException {
		s3Client = CommLib.buildS3Client();
	}

	@Test
	public void testUpdateDelimiter() throws Exception {
		try {
			DelimiterUtils.getDelimiter(bucketName);
		} catch (AmazonS3Exception e) {
			if (e.getStatusCode() != 404 && !e.getErrorCode().contains("NoSuchBucket")) {
				Assert.fail("get delimiter fail! e=" + e.getErrorCode() + e.getStatusCode());
			}
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
			}
		} finally {
			s3Client.shutdown();
		}
	}
}
