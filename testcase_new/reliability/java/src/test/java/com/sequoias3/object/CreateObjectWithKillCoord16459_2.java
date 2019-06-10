package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * test content: 开启版本控制，创建对象过程中db端节点异常
 * testlink-case: seqDB-16459
 * @author wangkexin
 * @Date 2019.01.09
 * @version 1.00
 */
public class CreateObjectWithKillCoord16459_2 extends SdbTestBase {
	private GroupMgr groupMgr = null;
	private String userName = "user16459";
	private String bucketName = "bucket16459";
	private String keyName = "key16459";
	private String roleName = "normal";
	private LinkedBlockingQueue<String> keyNames = new LinkedBlockingQueue<>();
	private Random random = new Random();
	private LinkedBlockingQueue<SaveMd5> md5 = new LinkedBlockingQueue<SaveMd5>();	
	private File localPath = null;
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private int count = 0;
	private boolean runSuccess = false;

	@BeforeClass(enabled=false)
	private void setUp() throws Exception {
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		
		groupMgr = GroupMgr.getInstance();
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		s3Client.createBucket(bucketName);
		CommLibS3.setBucketVersioning(s3Client, bucketName, "Enabled");
	}

	@Test(enabled=false)
	public void testCreateObject() throws Exception {
		try {
			GroupWrapper coordGroup = groupMgr.getGroupByName("SYSCoord");
            List<NodeWrapper> priNode = coordGroup.getNodes();
            System.out.println("Kill node:" + priNode.get(0).hostName() + ":" + priNode.get(0).svcName());

			FaultMakeTask faultTask = KillNode.getFaultMakeTask(priNode.get(0).hostName(), priNode.get(0).svcName(), 1);
			TaskMgr mgr = new TaskMgr(faultTask);
			
			CreateObjectTask oTask = new CreateObjectTask();
			mgr.addTask(oTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			checkCurrentObjectResult();
		} catch (ReliabilityException e) {
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}
		runSuccess = true;
	}

	@AfterClass(enabled=false)
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

	private class CreateObjectTask extends OperateTask {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
			PutObjectResult putObjResult = null;
			try {
				for (int i = 0; i < 100; i++) {
					int writeSize = random.nextInt(1024);
					String currContent = ObjectUtils.getRandomString(writeSize);
					String currmd5 = TestTools.getMD5(currContent.getBytes());
					String currentName = keyName + i ;
					putObjResult = s3Client.putObject(bucketName, currentName, currContent);
					md5.offer(new SaveMd5(currentName, putObjResult.getETag(), currmd5));
					keyNames.add(currentName);
					count++;
				}
			} /*catch (Exception e) {
            	System.out.println("the put object error i is =" + count); 
            }*/ finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
	
	private void checkCurrentObjectResult() throws Exception {
		Assert.assertEquals(keyNames.size(), 100, "keyNames size is wrong! ");
		for(int i = 0 ; i < 100; i++){
			String currentKeyName = keyNames.poll();
			GetObjectRequest request = new GetObjectRequest(bucketName, currentKeyName);
			S3Object object = s3Client.getObject(request);
			S3ObjectInputStream s3is = object.getObjectContent();		
			String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
					Thread.currentThread().getId());
			ObjectUtils.inputStream2File(s3is,downloadPath);
			s3is.close();
	        String actMd5 = TestTools.getMD5(downloadPath);
	        
			String expMd5 = getMd5ByKeyName(currentKeyName);
			Assert.assertEquals(actMd5, expMd5, "etag is wrong , key name is:" + currentKeyName);
		}
	}
	
	private String getMd5ByKeyName(String keyName){
		Iterator<SaveMd5> iterator = md5.iterator();
		boolean found = false;
		String findMd5 = new String();
		while(iterator.hasNext()){
			SaveMd5 current = iterator.next();
			String curkeyName = current.getKeyName();
			if(curkeyName.equals(keyName)){
				findMd5 = current.getMd5();
				md5.remove(current);
				found = true;
				break;
			}
		}
		if (!found) {
			Assert.fail("keyName[" + keyName + "] not found");
		}		
		return findMd5;
	}
	
	private class SaveMd5{
		private String keyName;
		private String md5;
		
		public SaveMd5(String keyName, String etag, String md5){
			this.keyName = keyName;
			this.md5 = md5;
		}
		
		public String getKeyName(){
			return keyName;
		}

		public String getMd5(){
			return md5;
		}
	}
}