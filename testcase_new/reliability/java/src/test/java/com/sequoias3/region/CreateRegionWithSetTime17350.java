package com.sequoias3.region;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.RegionUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import java.io.File;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

/**
 * test content:  创建区域过程中sdb节点所在主机时间跳变
 * testlink-case: seqDB-17350
 * @author wangkexin
 * @Date 2019.01.30
 * @version 1.00
 */

public class CreateRegionWithSetTime17350 extends S3TestBase {
	@DataProvider(name = "timeProvider")
	public Object[][] generateSetTime() {
		//in the past
        Calendar cal = Calendar.getInstance();
        //set time
        cal.set(Calendar.YEAR,cal.get(Calendar.YEAR)-1);
        cal.set(Calendar.MONTH,cal.get(Calendar.MONTH)-3);
        
        //in the future
        Calendar cal2 = Calendar.getInstance();
        System.out.println(cal2.get(Calendar.YEAR));
        //set time
        cal2.set(Calendar.YEAR,cal2.get(Calendar.YEAR)+1);
        cal2.set(Calendar.MONTH,cal2.get(Calendar.MONTH)+3);

		return new Object[][] {
				new Object[] {cal},
				new Object[] {cal2},
		};
	}
	
	private Calendar original_cal = Calendar.getInstance();
	private String regionName = "beijing17350";
	private String bucketName = "bucket17350";
	private String keyName = "key17350";
	private List<String> regionNames = new ArrayList<>();
	private int regionNum = 6;
	private int bucketNum = 6;
	private static Sequoiadb sdb = null;
	private AmazonS3 s3Client = null;
	private File localPath = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		s3Client = CommLibS3.buildS3Client();
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		
		clearBuckets();
		for(int i = 0; i < regionNum; i++ ){
			String currRegionName = regionName+"-"+i;
			regionNames.add(currRegionName);
			if(RegionUtils.headRegion(currRegionName)){
				RegionUtils.deleteRegion(currRegionName);
			}
		}
	}
	
	@Test(dataProvider = "timeProvider")
	public void testCreateRegion(Calendar cal) throws Exception {
        TaskMgr mgr = new TaskMgr();
        for(int i = 0; i < regionNum; i++) {
            mgr.addTask(new CreateRegion(regionName + "-" + i));
        }
        mgr.addTask(new SetTime(S3TestBase.s3HostName, cal.getTime().getTime()));
        mgr.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        
        for(int i = 0 ; i < bucketNum ; i++){
        	s3Client.createBucket(new CreateBucketRequest(bucketName + "-" + i, regionName+"-" + i));
        	s3Client.putObject(bucketName + "-" + i, keyName + "_" + i, "content17350");
        }
        
        checkCSCLresult(cal);
        checkObjects();
        
        //clear
        clearBuckets();
		deleteRegions(regionNames);
    	runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		try{
			if (runSuccess) {
				clearBuckets();
				deleteRegions(regionNames);
				sdb.close();
				TestTools.LocalFile.removeFile(localPath);
			}
		}catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			//recovery time
			TestTools.setSystemTime(S3TestBase.s3HostName, original_cal.getTime().getTime());
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
	
	private class SetTime extends OperateTask{
        private String host = null;
        private long date;

        public SetTime(String host, long date) {
            this.host = host;
            this.date = date;
        }

        @Override
        public void exec() throws Exception {
            TestTools.setSystemTime(host,date);
        }
    }
	
	private class CreateRegion extends OperateTask{
		private String regionName = null;
		public CreateRegion(String regionName){
			this.regionName = regionName;
		}
		@Override
		public void exec() throws Exception {
			Region region = new Region();
			region.withName(regionName);
			RegionUtils.putRegion(region);
		}
	}
	
	private void checkCSCLresult(Calendar cal){
		List<String> clNames = sdb.getCollectionNames();
		List<String> expCLNames = new ArrayList<>();
	    String expDataCLName = RegionUtils.getDataCLName("quarter", cal.getTime());
		for(int i = 0; i < regionNames.size(); i++ ){
		    String expDataCSName = RegionUtils.getDataCSName(regionNames.get(i), "year", cal.getTime());
		    String expMetaCSName = RegionUtils.getMetaCSName(regionNames.get(i));
		    expCLNames.add(expDataCSName + "." + expDataCLName);
		    expCLNames.add(expMetaCSName + ".S3_ObjectMeta");
		}
		
		Assert.assertTrue(clNames.containsAll(expCLNames));
	}
	
	private void checkObjects() throws Exception{
		for(int i = 0 ; i < bucketNum ; i++){
        	S3Object object = s3Client.getObject(bucketName + "-" + i, keyName + "_" + i);
        	Assert.assertEquals(object.getKey(), keyName + "_" + i);
        	String actMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName + "-" + i, keyName + "_" + i);
        	Assert.assertEquals(actMd5, TestTools.getMD5("content17350".getBytes()));
        }
	}
	
	private void clearBuckets(){
		for(int i = 0 ; i < bucketNum ; i++){
			CommLibS3.clearBucket(s3Client, bucketName + "-" + i);
		}
	}
	
	private void deleteRegions(List<String> regions) throws Exception{
		for(int i = 0; i < regions.size(); i++ ){
			if(RegionUtils.headRegion(regions.get(i))){
				RegionUtils.deleteRegion(regions.get(i));
			}
		}
	}
}
