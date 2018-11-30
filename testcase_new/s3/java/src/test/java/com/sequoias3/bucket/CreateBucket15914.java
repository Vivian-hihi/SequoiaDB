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
 * test content: remove empty bucket
 * testlink-case: seqDB-15914  
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15914 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket15914";
	private String userName = "user15914";
	private String roleName = "normal";
	private String delBucketName = "bucket15914.5";
	private final int defaultNums = 10;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		String[] accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test
	private void testCreateBucket() throws Exception {
		// create and delete buckets
		createBucketbyNums();
		checkCreateBucketResult(s3Client);
		s3Client.deleteBucket(delBucketName);

		// check buckets after delete
		checkResultAfterDelete(s3Client, delBucketName);
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
		for (int i = 1; i <= defaultNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
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

	private void checkResultAfterDelete(AmazonS3 s3Client, String bucketname) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums - 1);
		Assert.assertFalse(doesBucketExist(bucketname), "bucket still exist!");
	}

	private boolean doesBucketExist(String bucketName) {
		List<Bucket> buckets = s3Client.listBuckets();
		boolean findBucketFlag = false;
		for (int i = 0; i < buckets.size(); i++) {
			String actBucketName = buckets.get(i).getName();
			if (actBucketName.equals(bucketName)) {
				findBucketFlag = true;
				break;
			}
		}
		return findBucketFlag;
	}
}
