package com.sequoias3.delimiter;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content: 上传多个对象（对象名包含分隔符），更新分隔符 testlink-case: seqDB-18084
 * 
 * @author wangkexin
 * @Date 2019.04.12
 * @version 1.00
 */
public class UpdateDelimiter18084 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18084";
	private String userName = "user18084";
	private String roleName = "normal";
	private String delimiter1 = "/";
	private String delimiter2 = "%";
	private String[] keyList = null;
	private AmazonS3 s3Client = null;
	private String[] accessKeys = null;

	@DataProvider(name = "keyNumsProvider")
	public Object[][] recordNumsProvider() {
		// TODO:常量数据请给出描述信息和具体含义
		return new Object[][] { { 5 }, { 10 }, { 15 }, };
	}

	@BeforeClass
	private void setUp() throws Exception {
	}

	@Test(dataProvider = "keyNumsProvider")
	private void testUpdateDelimiter(int keyNum) throws Exception {
		// TODO:创建用户和桶没有必要每次操作都创建再删除
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
		s3Client.createBucket(bucketName);
		// TODO:key名建议带上用例ID，如果出问题方便在日志中查看
		keyList = DelimiterUtils.getRandomKeyListWithDelimiter(delimiter1, delimiter2, keyNum);
		for (int i = 0; i < keyList.length; i++) {
			s3Client.putObject(bucketName, keyList[i], "test18084");
		}

		// 更新分隔符为delimiter2并检查结果(这里通过携带delimiter查询对象列表的对外映射场景检测目录表是否生成新目录，对象元数据表和目录表中数据通过连接db手工校验)
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter2, accessKeys[0]);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter2, accessKeys[0]);

		List<String> expCommonPrefixes = ObjectUtils.getCommPrefixes(keyList, "", delimiter2);
		DelimiterUtils.listObjectsWithDelimiter(s3Client, bucketName, delimiter2, expCommonPrefixes,
				new ArrayList<String>());
		runSuccess = true;
		if (runSuccess) {
			UserUtils.deleteUser(userName);
		}
	}

	@AfterClass
	private void tearDown() throws Exception {
		if (s3Client != null) {
			s3Client.shutdown();
		}
	}
}
