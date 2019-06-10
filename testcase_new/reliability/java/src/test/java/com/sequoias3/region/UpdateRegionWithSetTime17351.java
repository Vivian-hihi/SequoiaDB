package com.sequoias3.region;


import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.bean.GetRegionResult;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.RegionUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.*;


/**
 * @Description seqDB-17342 ::更新区域过程中db端节点异常
 * @author fanyu
 * @version 1.00
 * @Date 2019.01.29
 */

public class UpdateRegionWithSetTime17351 extends S3TestBase {
    private boolean runSuccess = false;
    private int regionNum = 3;
    private String regionNameBase = "region17342a";
    private String dataCSShardingType = "year";
    private String dataCLShardingType = "year";
    private String updateDataCSShardingType = "year";
    private String updateDataCLShardingType = "month";
    private AmazonS3 s3Client = null;
    private String bucketName = "bucket17351a";
    private String objectName = "object17351a";
    private List<String> regionNames = new ArrayList<String>();
    Calendar cal = Calendar.getInstance();
    private Date oldDate;
    private Date newDate;

    @BeforeClass
    private void setUp() throws Exception {
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client,bucketName);
        oldDate = cal.getTime();
        cal.set(Calendar.MONTH,cal.get(Calendar.MONTH)+1);
        newDate = cal.getTime();
        for (int i = 0; i < regionNum; i++) {
            CommLibS3.clearBucket(s3Client,bucketName+i);
            RegionUtils.deleteRegion(regionNameBase + i);
            Region region = new Region().withDataCSShardingType(dataCSShardingType)
                    .withDataCLShardingType(dataCLShardingType)
                    .withName(regionNameBase + i);
            RegionUtils.putRegion(region);
            s3Client.createBucket(new CreateBucketRequest(bucketName+i, regionNameBase + i));
            s3Client.putObject(bucketName+i,objectName+i,String.valueOf(UUID.randomUUID()));
            regionNames.add(regionNameBase + i);
        }
    }

    @Test
    public void test() throws Exception {
        //update region when set time
        TaskMgr mgr = new TaskMgr();
        mgr.addTask(new SetTime(S3TestBase.s3HostName,newDate.getTime()));
        for (int i = 0; i < regionNum; i++) {
            mgr.addTask(new PutRegion(regionNames.get(i),updateDataCSShardingType,updateDataCLShardingType));
        }
        mgr.execute();
        Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

        //get region that has been updated and check
        int index = new Random().nextInt(regionNames.size());
        String regionName = regionNames.get(index);

        //check region information
        GetRegionResult result1 = RegionUtils.getRegion(regionName);
        Assert.assertEquals(result1.getBuckets().size(), 1, result1.getBuckets().toString());
        Assert.assertEquals(result1.getBuckets().get(0).getName(), bucketName+index);
        Region region1 = result1.getRegion();
        Assert.assertEquals(region1.getDataCSShardingType(),updateDataCSShardingType);
        Assert.assertEquals(region1.getDataCLShardingType(),updateDataCLShardingType);

       // //check old object information
        S3Object object = s3Client.getObject(bucketName+index,objectName+index);
        Assert.assertEquals(object.getKey(),objectName+index);
        Assert.assertEquals(object.getBucketName(),bucketName+index);
        ObjectMetadata objectMetadata = object.getObjectMetadata();
        Assert.assertEquals(objectMetadata.getVersionId(),"null");

        String dataCSName = RegionUtils.getDataCSName(regionName,dataCSShardingType,oldDate);
        String dataClName =  RegionUtils.getDataCLName(dataCLShardingType,oldDate);
        Assert.assertEquals(  RegionUtils.getRecordNum(dataCSName,dataClName),1);

        //put new object  and check
        s3Client.putObject(bucketName+index,objectName+"new" + index,String.valueOf(UUID.randomUUID()));
        s3Client.getObject(bucketName+index,objectName+"new" + index);
        String dataCSName1 = RegionUtils.getDataCSName(regionName,updateDataCSShardingType,newDate);
        String dataClName1 =  RegionUtils.getDataCLName(updateDataCLShardingType,newDate);
        Assert.assertEquals(  RegionUtils.getRecordNum(dataCSName1,dataClName1),1);

        //in the past
        cal.set(Calendar.MONTH,cal.get(Calendar.MONTH) - 3);
        Date newDate1 = cal.getTime();
        TestTools.setSystemTime(S3TestBase.s3HostName,newDate1.getTime());
        s3Client.putObject(bucketName+index,objectName+"new1" + index,String.valueOf(UUID.randomUUID()));
        s3Client.getObject(bucketName+index,objectName+"new1" + index);
        String dataCSName2 = RegionUtils.getDataCSName(regionName,updateDataCSShardingType,newDate1);
        String dataCLName2 =  RegionUtils.getDataCLName(updateDataCLShardingType,newDate1);
        Assert.assertEquals( RegionUtils.getRecordNum(dataCSName2,dataCLName2),1,"objectName = " + objectName+"new1" + index);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() throws Exception {
        TestTools.restoreSystemTime(S3TestBase.s3HostName);
        if(runSuccess) {
            for (int i = 0; i < regionNum; i++) {
                CommLibS3.clearBucket(s3Client, bucketName+i);
                RegionUtils.deleteRegion(regionNameBase + i);
            }
        }
    }

    private class SetTime extends OperateTask {
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

    private class PutRegion extends OperateTask {
        private String regionName = null;
        private String dataCSShardingType = null;
        private String dataCLShardingType = null;

        public PutRegion(String regionName, String dataCSShardingType, String dataCLShardingType) {
            this.regionName = regionName;
            this.dataCSShardingType = dataCSShardingType;
            this.dataCLShardingType = dataCLShardingType;
        }

        @Override
        public void exec() throws Exception {
            Region region = new Region().withDataCSShardingType(dataCSShardingType)
                    .withDataCLShardingType(dataCLShardingType)
                    .withName(regionName);
            RegionUtils.putRegion(region);
        }
    }
}
