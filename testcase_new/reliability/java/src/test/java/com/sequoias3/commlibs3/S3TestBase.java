package com.sequoias3.commlibs3;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.testng.Assert;
import org.testng.annotations.AfterSuite;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Parameters;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

public class S3TestBase {
	public static String coordUrl;
	public static String hostName;
	public static String serviceName;
	public static String s3ClientUrl;
	public static String s3HostName;
	public static String s3Port;
	public static String csName;
	public static String bucketName;
	public static String enableVerBucketName;
	public static int reservedPortBegin;
	public static int reservedPortEnd;
	public static String reservedDir;
	public static String workDir;
	public static String s3UserName;
	public static String s3AccessKeyId;
	public static String confTool;
	public static String rootPwd;
	public static String remoteUser;
	public static String remotePwd;
	public static String scriptDir;
	private static SdbConfTestBase sdbConfTestBase = new SdbConfTestBase();
	protected static String installPath;

	public static final String PARTLISTINUSEOFF = "partlistinuseoff";
	public static final String PARTSIZELIMITOFF = "partsizelimitoff";
	private static final String PARTLISTINUSE = "sdbs3.multipartupload.partlistinuse=";
	private static final String PARTSIZELIMIT = "sdbs3.multipartupload.partsizelimit=";

	private static String propertiesFileName = "";
	private static String replaceFileName = "";
	private static String testGroup = null;
	private static final Map<String, String> group2Conf = new HashMap<String, String>();

	static {
		group2Conf.put(PARTLISTINUSEOFF, PARTLISTINUSE + "false");
		group2Conf.put(PARTSIZELIMITOFF, PARTSIZELIMIT + "false");
	}

	public static synchronized void setRunGroup(List<String> testGroups) {
		if (testGroups.size() != 1) {
			return;
		}
		if (!testGroups.get(0).equals(S3TestBase.testGroup)) {
			S3TestBase.testGroup = testGroups.get(0);
		}
	}

	@Parameters({ "HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN", "RSRVPORTEND", "RSRVNODEDIR", "WORKDIR",
			"S3HOSTNAME", "S3PORT", "S3USERNAME", "S3ACCESSKEYID", "CONFTOOL", "ROOTPASSWD", "REMOTEUSER",
			"REMOTEPASSWD", "SCRIPTDIR" })
	@BeforeSuite(alwaysRun = true)
	public static void initSuite(String HOSTNAME, String SVCNAME, String COMMCSNAME, int RSRVPORTBEGIN, int RSRVPORTEND,
			String RSRVNODEDIR, String WORKDIR, String S3HOSTNAME, String S3PORT, String S3USERNAME,
			String S3ACCESSKEYID, String CONFTOOL, String ROOTPASSWD, String REMOTEUSER, String REMOTEPASSWD,
			String SCRIPTDIR) throws Exception {
		hostName = HOSTNAME;
		serviceName = SVCNAME;
		csName = COMMCSNAME;
		reservedPortBegin = RSRVPORTBEGIN;
		reservedPortEnd = RSRVPORTEND;
		reservedDir = RSRVNODEDIR;
		workDir = WORKDIR;
		coordUrl = HOSTNAME + ":" + SVCNAME;
		s3HostName = S3HOSTNAME;
		s3Port = S3PORT;
		s3UserName = S3USERNAME;
		s3AccessKeyId = S3ACCESSKEYID;
		s3ClientUrl = "http://" + S3HOSTNAME + ":" + S3PORT;
		bucketName = "commbucket";
		enableVerBucketName = "commbucketwithversion";
		confTool = CONFTOOL;
		rootPwd = ROOTPASSWD;
		remoteUser = REMOTEUSER;
		remotePwd = REMOTEPASSWD;
		scriptDir = SCRIPTDIR;

		getInstallPath();

		sdbConfTestBase.openTransaction(confTool, hostName, serviceName);
		changeConfAndStartS3();
		// clean file
		File workDirFile = new File(workDir);
		if (!workDirFile.exists()) {
			workDirFile.mkdir();
		}

		Sequoiadb db = null;
		try {
			db = new Sequoiadb(coordUrl, "", "");
			boolean ret = createCommonCS(db);
			Assert.assertTrue(ret);
		} catch (BaseException e) {
			Assert.fail("connect " + coordUrl + ": " + e.getErrorCode());
		} finally {
			if (db != null) {
				db.close();
			}
		}

		AmazonS3 s3Client = null;
		try {
			// clean up existing buckets
			s3Client = CommLibS3.buildS3Client();
			List<Bucket> buckets = s3Client.listBuckets();
			for (int i = 0; i < buckets.size(); i++) {
				String bucketName = buckets.get(i).getName();
				String bucketVerStatus = s3Client.getBucketVersioningConfiguration(bucketName).getStatus();
				if (bucketVerStatus == "null") {
					CommLibS3.deleteAllObjects(s3Client, bucketName);
				} else {
					CommLibS3.deleteAllObjectVersions(s3Client, bucketName);
				}
				s3Client.deleteBucket(bucketName);
			}

			// create bucket
			s3Client.createBucket(new CreateBucketRequest(bucketName));

			// create bucket by enable versioning
			s3Client.createBucket(new CreateBucketRequest(enableVerBucketName));
			CommLibS3.setBucketVersioning(s3Client, enableVerBucketName, "Enabled");
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}

	}

	@BeforeTest(groups = { PARTLISTINUSEOFF, PARTLISTINUSEOFF }, alwaysRun = true)
	public static synchronized void initTestGroups() throws Exception {
		if (testGroup == null)
			return;
		System.out.println("init " + testGroup + " Groups...........");
		String config = group2Conf.get(testGroup);
		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec("echo '" + config + "' >>" + propertiesFileName);
			if (ssh.getExitStatus() != 0) {
				throw new Exception(
						"exec echo '" + config + "' >>" + propertiesFileName + " failed, stout= " + ssh.getStdout());
			}
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
		}
		stopS3();
		startS3();
	}

	@AfterTest(groups = { PARTLISTINUSEOFF, PARTLISTINUSEOFF }, alwaysRun = true)
	public static synchronized void finiTestGroups() throws Exception {
		if (testGroup == null)
			return;
		System.out.println("fini " + testGroup + " Groups...........");
		String config = group2Conf.get(testGroup);
		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec("sed -i 's/" + config + "/" + "#" + config + "/g' " + propertiesFileName);
			if (ssh.getExitStatus() != 0) {
				throw new Exception("exec sed -i 's/" + config + "/" + "#" + config + "/g' " + propertiesFileName
						+ " failed, stout= " + ssh.getStdout());
			}
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
		}
		testGroup = null;
		stopS3();
		startS3();
	}

	@AfterSuite(alwaysRun = true)
	public static void finiSuite() throws Exception {
		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec("rm -f " + propertiesFileName + ";" + "mv " + replaceFileName + " " + propertiesFileName);
			System.out.println("restore properties: " + propertiesFileName);
			if (ssh.getExitStatus() != 0) {
				throw new Exception("exec rm -f " + propertiesFileName + ";" + "mv " + replaceFileName + " "
						+ propertiesFileName + " failed in the end, stout= " + ssh.getStdout());
			}
			getClusterInfo();
			sdbConfTestBase.closeTransaction(hostName, serviceName);
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
			stopS3();
		}
	}

	public static String getDefaultCoordUrl() {
		return coordUrl;
	}

	public static String getWorkDir() {
		return workDir;
	}

	public static String getDefaultS3ClientUrl() {
		return s3ClientUrl;
	}

	public static void changeConfAndStartS3() throws Exception {
		propertiesFileName = installPath + "/tools/sequoias3/config/application.properties";
		replaceFileName = installPath + "/tools/sequoias3/config/ori_application.properties";
		String logBackFileName = installPath + "/tools/sequoias3/config/logback.xml";
		// 更新properties
		System.out.println("begin update application.properties");
		Sequoiadb localdb = null;
		try {
			localdb = new Sequoiadb(coordUrl, "", "");
			ReplicaGroup rg = localdb.getReplicaGroup("SYSCoord");
			BSONObject rgDetail = rg.getDetail();
			BasicBSONList groupInfo = (BasicBSONList) rgDetail.get("Group");
			String coordUrls = "";
			for (int i = 0; i < groupInfo.toArray().length; i++) {
				BSONObject groupObj = (BSONObject) groupInfo.toArray()[i];
				String groupName = (String) groupObj.get("HostName");
				if (i != 0) {
					coordUrls = coordUrls + ",";
				}
				coordUrls = coordUrls + groupName + ":" + serviceName;
			}

			Ssh ssh = null;
			try {
				ssh = new Ssh(s3HostName, remoteUser, remotePwd);
				ssh.exec("mv " + propertiesFileName + " " + replaceFileName + ";" + "touch " + propertiesFileName + ";"
						+ "echo 'sdbs3.sequoiadb.url=sequoiadb://" + coordUrls + "' >" + propertiesFileName);
				System.out.println("write properties: " + propertiesFileName);
				if (ssh.getExitStatus() != 0) {
					throw new Exception("exec update application.properties file failed in the beginning, stout= "
							+ ssh.getStdout());
				}

				// change log level
				ssh.exec("sed -i 's/INFO/DEBUG/g' " + logBackFileName);
				if (ssh.getExitStatus() != 0) {
					throw new Exception("exec change log level failed in the beginning, stout= " + ssh.getStdout());
				}
			} finally {
				if (ssh != null) {
					ssh.disconnect();
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			Assert.fail("update application.properties file failed");
		} finally {
			if (localdb != null) {
				localdb.close();
			}
		}
		System.out.println("finish update application.properties");
		startS3();
	}

	public static void getInstallPath() throws Exception {
		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec("cat /etc/default/sequoiadb | grep 'INSTALL_DIR'");
			if (ssh.getExitStatus() != 0) {
				throw new Exception(
						"exec cat /etc/default/sequoiadb | grep 'INSTALL_DIR' failed, stout= " + ssh.getStdout());
			}
			String installDirConf = ssh.getStdout();
			installPath = installDirConf.substring(installDirConf.lastIndexOf("=") + 1, installDirConf.length() - 1);
		} catch (Exception e) {
			e.printStackTrace();
			Assert.fail("get s3 server installPath failed");
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
		}
	}

	public static void getClusterInfo() throws IOException {
		String clusterFileName = installPath + "/tools/sequoias3/log/cluster.log";
		String info = "";
		Sequoiadb db = null;
		try {
			db = new Sequoiadb(coordUrl, "", "");
			DBCursor cur = db.getList(7, null, null, null);
			while (cur.hasNext()) {
				info += cur.getNext().toString();
			}
			cur.close();
		} catch (BaseException e) {
			Assert.fail("connect " + coordUrl + " get cluster info error : " + e.getErrorCode());
		} finally {
			if (db != null) {
				db.close();
			}
		}

		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec("touch " + clusterFileName + ";" + "echo " + info + " >" + clusterFileName);
			if (ssh.getExitStatus() != 0) {
				throw new Exception("exec touch " + clusterFileName + ";" + "echo " + info + " >" + clusterFileName
						+ " failed, stout= " + ssh.getStdout());
			}
		} catch (Exception e) {
			e.printStackTrace();
			Assert.fail("write cluster info failed");
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
		}
	}

	public static void startS3() throws Exception {
		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec("source /etc/profile;" + installPath + "/tools/sequoias3/sequoias3.sh start " + "> " + installPath
					+ "/tools/sequoias3/s3start.log");
			if (ssh.getExitStatus() != 0) {
				throw new Exception("exec command : source /etc/profile;" + installPath
						+ "/tools/sequoias3/sequoias3.sh start" + "failed,stout= " + ssh.getStdout());
			}
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
		}
	}

	public static void stopS3() throws Exception {
		Ssh ssh = null;
		try {
			ssh = new Ssh(s3HostName, remoteUser, remotePwd);
			ssh.exec(installPath + "/tools/sequoias3/sequoias3.sh stop -a");
			if (ssh.getExitStatus() != 0) {
				throw new Exception("exec command : " + installPath + "/tools/sequoias3/sequoias3.sh stop -a "
						+ "failed,stdout= " + ssh.getStdout() + ",stderr = " + ssh.getStderr());
			}
		} finally {
			if (ssh != null) {
				ssh.disconnect();
			}
		}

		Sequoiadb db = null;
		try {
			db = new Sequoiadb(coordUrl, "", "");
			if (db.isCollectionSpaceExist(csName)) {
				db.dropCollectionSpace(csName);
			}
		} catch (BaseException e) {
			e.printStackTrace();
		} finally {
			if (db != null) {
				db.close();
			}
		}
	}

	private static boolean createCommonCS(Sequoiadb sdb) {
		boolean isCreateSuccess = true;
		try {
			if (sdb.isCollectionSpaceExist(csName)) {
				sdb.dropCollectionSpace(csName);
			}
			sdb.createCollectionSpace(csName);
		} catch (BaseException e) {
			System.out.printf("create CollectionSpace %s failed, errMsg:%s\n", csName, e.getMessage());
			isCreateSuccess = false;
		}
		return isCreateSuccess;
	}
}
