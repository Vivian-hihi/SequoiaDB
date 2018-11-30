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
 * test content: delete some buckets and get bucket list information
 * testlink-case: seqDB-15911  
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15911 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket15911";
	private String userName = "user15911";
	private String roleName = "normal";
	private final int defaultNums = 10;
	private final int leftInterval = 1;
	private final int rightInterval = 8;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		String[] accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test
	public void testCreateBucket() throws Exception {
		// create and delete buckets
		createBucketbyNums();
		checkCreateBucketResult(s3Client);
		deleteBuckets();

		// check buckets after delete
		checkResultAfterDelete(s3Client);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
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
		for (int i = 1; i <= defaultNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
		}
	}

	private void deleteBuckets() {
		for (int i = leftInterval; i <= rightInterval; i++) {
			String delBucketName = bucketName + "." + i;
			s3Client.deleteBucket(delBucketName);
		}
	}

	private void checkCreateBucketResult(AmazonS3 s3Client) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums);

		for (int index = 0; index < buckets.size(); index++) {
			Bucket bucket = buckets.get(index);
			String actBucketName = bucket.getName();
			String actOwner = bucket.getOwner().getDisplayName();
			Assert.assertEquals(actBucketName, bucketName + "." + (index + 1));
			Assert.assertEquals(actOwner, userName);
		}
	}

	private void checkResultAfterDelete(AmazonS3 s3Client) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums - (rightInterval - leftInterval + 1));

		for (int index = 0; index < buckets.size(); index++) {
			Bucket bucket = buckets.get(index);
			String actBucketName = bucket.getName();
			String actOwner = bucket.getOwner().getDisplayName();
			if (index < leftInterval - 1) {
				Assert.assertEquals(actBucketName, bucketName + "." + (index + 1));
				Assert.assertEquals(actOwner, userName);
			} else {
				Assert.assertEquals(actBucketName, bucketName + "." + (index + 1 + (rightInterval - leftInterval + 1)));
				Assert.assertEquals(actOwner, userName);
			}
		}
	}
}
