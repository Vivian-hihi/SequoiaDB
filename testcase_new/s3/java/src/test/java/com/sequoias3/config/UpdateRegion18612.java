package com.sequoias3.config;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.region.GetRegionResult;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 更新区域配置lobPageSize和replSize testlink-case: seqDB-18612
 * 
 * @author wangkexin
 * @Date 2019.06.27
 * @version 1.00
 */
public class UpdateRegion18612 extends S3TestBase {
    @DataProvider(name = "lobPageSizeAndReplSize")
    public Object[][] generatelobPageSizeAndReplSize() {
        return new Object[][] { new Object[] { "8192", "", "4096", "", "ConflictLobPageSize" },
                new Object[] { "", "1", "", "2", "ConflictReplSize" },
                new Object[] { "16384", "3", "65536", "4", "ConflictLobPageSize" } };
    }

    private String regionName = "region18612";
    private String defaultDataLobPageSize = "262144";
    private String defaultDataReplSize = "-1";

    @BeforeClass
    private void setUp() throws Exception {
        RegionUtils.clearRegion(regionName);
    }

    @Test(dataProvider = "lobPageSizeAndReplSize")
    private void testUpdateRegion(String dataLobPageSize, String dataReplSize, String upDataLobPageSize,
            String upDataReplSize, String errorCode) throws Exception {
        // create region
        Region region = new Region();
        region.withDataCSShardingType("year").withDataCLShardingType("year").withName(regionName)
                .withDataLobPageSize(dataLobPageSize).withDataReplSize(dataReplSize);
        RegionUtils.putRegion(region);

        region.withDataCSShardingType("year").withDataCLShardingType("year").withName(regionName)
                .withDataLobPageSize(upDataLobPageSize).withDataReplSize(upDataReplSize);

        try {
            // update region
            RegionUtils.putRegion(region);
            Assert.fail("exp failed but act success,region = " + region.toString());
        } catch (AmazonS3Exception e) {
            Assert.assertEquals(e.getErrorCode(), errorCode);
        }
        checkLobPageSizeAndReplSize(dataLobPageSize, dataReplSize);
        RegionUtils.deleteRegion(regionName);
    }

    @AfterClass
    private void tearDown() throws Exception {
    }

    private void checkLobPageSizeAndReplSize(String dataLobPageSize, String dataReplSize) throws Exception {
        GetRegionResult result = RegionUtils.getRegion(regionName);
        Region region = result.getRegion();
        if (dataLobPageSize.equals("")) {
            dataLobPageSize = defaultDataLobPageSize;
        }
        if (dataReplSize.equals("")) {
            dataReplSize = defaultDataReplSize;
        }
        Assert.assertEquals(region.getDataLobPageSize(), dataLobPageSize);
        Assert.assertEquals(region.getDataReplSize(), dataReplSize);
    }
}
