package com.sequoias3.bucket;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.Owner;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * test content: concurrent get bucket information
 * testlink-case: seqDB-15912
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15912 extends S3TestBase {
	private boolean runSuccess = false;
	private String userName = "user15912";
	private String bucketName = "bucket15912";
	private String roleName = "normal";
	private final int defaultNums = 30;
	private List<String> expBucketNameList = new ArrayList<String>();
	private AmazonS3 s3Client = null;
	private String[] acessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);
		createBuckets(s3Client);
	}

	@Test
	private void testGetBuckets() throws Exception {
		GetBucketThread getBuckets = new GetBucketThread();
		getBuckets.start(100);
		Assert.assertTrue(getBuckets.isSuccess(), getBuckets.getErrorMsg());

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

	private class GetBucketThread extends S3ThreadBase {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);
			try {
				s3Client.listBuckets();
				checkBucketResult(s3Client);
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}

	private void createBuckets(AmazonS3 s3Client) {
		for (int i = 0; i < defaultNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
			expBucketNameList.add(subBucketName);
		}
	}

	private void checkBucketResult(AmazonS3 s3Client) {
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums);
		
		List<String> actbucketNameLists = new ArrayList<>();
		for(Bucket bucket : buckets){
			Owner actOwner = bucket.getOwner();
			Assert.assertEquals(actOwner.getDisplayName(), userName);
			actbucketNameLists.add(bucket.getName());
		}
		Collections.sort(actbucketNameLists);
		Collections.sort(expBucketNameList);
		for(int i = 0 ; i < actbucketNameLists.size() ; i++){
			Assert.assertEquals(actbucketNameLists.get(i),expBucketNameList.get(i));
		}
	}
}
