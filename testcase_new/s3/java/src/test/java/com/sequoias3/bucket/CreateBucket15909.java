package com.sequoias3.bucket;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.List;

/**
 * test content: create multiple buckets to get bucket list information
 * testlink-case: seqDB-15909
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15909 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket15909";
	private String userName = "user15909";
	private String roleName = "normal";
	private final int defaultNums = 100;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		String[] accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test
	private void testCreateBucket() throws Exception {
		// create buckets
		createBucketbyNums();
		checkCreateBucketResult(s3Client);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBuckets(s3Client);
				UserUtils.deleteUser(userName);
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void createBucketbyNums() {
		for (int i = 0; i < defaultNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
		}
	}

	private void checkCreateBucketResult(AmazonS3 s3Client) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums);

		for (int i = 0; i < buckets.size(); i++) {
			Bucket bucket = buckets.get(i);
			String actBucketName = bucket.getName();
			String actOwner = bucket.getOwner().getDisplayName();
			Assert.assertEquals(actBucketName, bucketName + "." + i);
			Assert.assertEquals(actOwner, userName);
		}
	}
}
