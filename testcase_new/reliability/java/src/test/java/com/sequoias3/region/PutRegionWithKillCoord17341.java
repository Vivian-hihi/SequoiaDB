package com.sequoias3.region;


import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.bean.GetRegionResult;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.RegionUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CopyOnWriteArrayList;


/**
 * @Description seqDB-16472 ::开启版本控制，创建对象过程中SequoiaS3和sdb节点网络异常
 * @author fanyu
 * @version 1.00
 * @Date 2019.01.29
 */


public class PutRegionWithKillCoord17341 extends S3TestBase {
    private boolean runSuccess = false;
    private int regionNum = 100;
    private String regionNameBase = "region17342a";
    private String dataCSShardingType = "year";
    private String dataCLShardingType = "month";
    private List<String> regionNames = new ArrayList<String>();
    private List<String> regionNameList = new CopyOnWriteArrayList<String>();

    @BeforeClass
    private void setUp() throws Exception {
        for (int i = 0; i < regionNum; i++) {
            RegionUtils.deleteRegion(regionNameBase + i);
            regionNames.add(regionNameBase + i);
        }
    }

    @Test
    public void test() throws Exception {
        //put region when kill coord
        FaultMakeTask faultTask = KillNode.getFaultMakeTask(SdbTestBase.hostName, SdbTestBase.serviceName, 1);
        TaskMgr mgr = new TaskMgr(faultTask);
        for (int i = 0; i < regionNum; i++) {
            mgr.addTask(new PutRegion(regionNames.get(i)));
        }
        mgr.execute();
        Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());
        System.out.println("regionNameList = " + regionNameList.size());

        //put region again
        regionNames.removeAll(regionNameList);

        for (String regionName : regionNames) {
            Region region = new Region().withDataCSShardingType(dataCSShardingType)
                    .withDataCLShardingType(dataCLShardingType)
                    .withName(regionName);
            RegionUtils.putRegion(region);
        }

        int index = new Random().nextInt(regionNum);
        String regionName = regionNameBase + index;
        System.out.println("regionName = " + regionName);
        GetRegionResult result = RegionUtils.getRegion(regionName);
        Assert.assertEquals(result.getBuckets().size(), 0, result.getBuckets().toString());
        Region region = result.getRegion();
        Assert.assertEquals(region.getDataCSShardingType(), dataCSShardingType);
        Assert.assertEquals(region.getDataCLShardingType(), dataCLShardingType);
        runSuccess = true;
    }


    @AfterClass
    private void tearDown() throws Exception {
        if(runSuccess) {
            for (int i = 0; i < regionNum; i++) {
                RegionUtils.deleteRegion(regionNameBase + i);
            }
        }
    }

    private class PutRegion extends OperateTask {
        private String regionName = null;

        public PutRegion(String regionName) {
            this.regionName = regionName;
        }

        @Override
        public void exec() throws Exception {
            Region region = new Region().withDataCSShardingType(dataCSShardingType)
                    .withDataCLShardingType(dataCLShardingType)
                    .withName(regionName);
            try {
                RegionUtils.putRegion(region);
                regionNameList.add(this.regionName);
            } catch (AmazonS3Exception e) {
                if (e.getStatusCode() != 500 && !e.getErrorCode().contains("GetDBConnectFail")) {
                    throw e;
                }
            }
        }
    }
}
