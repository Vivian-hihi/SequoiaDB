package com.sequoias3.delimiter;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;

/**
 * test content: 存在多个deleting状态分隔符，清理分隔符 testlink-case: seqDB-18086
 * 
 * @author wangkexin
 * @Date 2019.04.23
 * @version 1.00
 */
// TODO:这个自动化用例是没有覆盖测试点的，没有意义，需要创建多个更新分隔符任务，结合定时任务清理时间去验证
public class UpdateDelimiter18086 extends S3TestBase {
	private String bucketName = "bucket18086";
	private int bucketNum = 20;
	private List<String> bucketList = new ArrayList<>();
	private String delimiter = "test";
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		for (int i = 0; i < bucketNum; i++) {
			String curBucketName = bucketName + i;
			s3Client.createBucket(new CreateBucketRequest(curBucketName));
			bucketList.add(curBucketName);
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		ThreadExecutor es = new ThreadExecutor();
		for (String bucket : bucketList) {
			es.addWorker(new TransUpdateDelimiter18086(bucket));
		}
		es.run();
		// 手工查看到达清理任务周期后原分隔符最终被删除，对应taskId已不存在
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				for (String bucketName : bucketList) {
					s3Client.deleteBucket(bucketName);
				}
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	class TransUpdateDelimiter18086 {
		private String bucketName = "";

		public TransUpdateDelimiter18086(String bucketName) {
			this.bucketName = bucketName;
		}

		@ExecuteOrder(step = 1, desc = "更新分隔符")
		public void updateDelimiter() throws Exception {
			DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
			DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);
		}
	}
}
