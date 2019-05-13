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
 * test content: 更新分隔符，指定不同格式分隔符 testlink-case: seqDB-18089
 * 
 * @author wangkexin
 * @Date 2019.04.12
 * @version 1.00
 */
public class UpdateDelimiter18089 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18089";
	private String userName = "user18089";
	private String roleName = "normal";
	private String oldDelimiter = "/";
	private int keyNum = 10;
	private String[] objectNames = new String[keyNum];
	private AmazonS3 s3Client = null;
	private String[] accessKeys = null;

	@DataProvider(name = "delimitersProvider")
	public Object[][] recordNumsProvider() {
		String ascii = new String();
		for (int i = 0; i < 32; i++) {
			ascii += (char) i;
		}
		ascii += (char) 127;

		return new Object[][] {
				// test a : 包含 数字字符[0-9a-zA-Z]
				new Object[] { "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" },
				// test b : 包含特殊字符
				new Object[] { "!-_.*'()" },
				// test c : 包含需要特殊处理的字符 SEQUOIADBMAINSTREAM-4392
				// new Object[] {"&@:,$=+?;" + ascii + " "},
				// test d : 包含不建议使用的字符
				new Object[] { "\\^`><{}[]#%“~|" },
				// test e : 包含中文字符
				new Object[] { "测试分隔符" } };
	}

	@BeforeClass
	private void setUp() throws Exception {
	}

	@Test(dataProvider = "delimitersProvider")
	private void testUpdateDelimiter(String newDelimiter) throws Exception {
		// TODO:1、用例中建议创建一次用户，没有必要每次都创建/新增用户，桶也可以考虑只创建一次
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
		s3Client.createBucket(bucketName);
		// TODO:2、创建对象建议加上用例ID，方便区分不同用例中的key
		objectNames = DelimiterUtils.getRandomKeyListWithDelimiter(oldDelimiter, newDelimiter, keyNum);
		for (int i = 0; i < objectNames.length; i++) {
			s3Client.putObject(bucketName, objectNames[i], "test18089");
		}

		// 更新分隔符为delimiter2并检查结果(这里通过携带delimiter查询对象列表的对外映射场景检测目录表是否生成新目录，对象元数据表和目录表中数据通过连接db手工校验)
		DelimiterUtils.putBucketDelimiter(bucketName, newDelimiter, accessKeys[0]);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, newDelimiter, accessKeys[0]);

		List<String> expCommonPrefixes = ObjectUtils.getCommPrefixes(objectNames, "", newDelimiter);
		List<String> expContents = new ArrayList<>();
		DelimiterUtils.listObjectsWithDelimiter(s3Client, bucketName, newDelimiter, expCommonPrefixes, expContents);
		runSuccess = true;
		if (runSuccess) {
			UserUtils.deleteUser(userName);
		}
	}

	// TODO:3、tearDown中关闭连接只需要调用一次
	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
