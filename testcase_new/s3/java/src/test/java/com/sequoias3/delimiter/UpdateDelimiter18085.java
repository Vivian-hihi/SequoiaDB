package com.sequoias3.delimiter;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content: 定时清理分隔符 testlink-case: seqDB-18085
 * 
 * @author wangkexin
 * @Date 2019.04.12
 * @version 1.00
 */
// TODO:该用例只是验证分隔符更新结果，没有必要实现自动化，重复代码，需要手工等待任务检测周期到达去验证，手工验证时建议多个分隔符清理一起验证
public class UpdateDelimiter18085 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18085";
	private String userName = "user18085";
	private String roleName = "normal";
	private String delimiter = "%";
	private AmazonS3 s3Client = null;
	private String[] accessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
		s3Client.createBucket(bucketName);
	}

	@Test
	private void testUpdateDelimiter() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, accessKeys[0]);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter, accessKeys[0]);

		// 打断点在tearDown之前连接db手工校验原分隔符(/)状态以及到达定时清理任务周期后的原分隔符状态
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
