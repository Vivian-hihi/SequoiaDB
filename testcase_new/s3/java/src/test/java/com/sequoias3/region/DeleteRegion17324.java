package com.sequoias3.region;

import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;


/**
 * @Description: seqDB-17324:删除空区域
 * @author fanyu
 * @Date:2019年01月22日
 * @version:1.0
 */
public class DeleteRegion17324 extends S3TestBase {
    private String regionName1 = "region17324a";
    private String regionName2 = "region17324b";
    private String csName = "cs17324";
    private String[] clNames = {"dataCL17302A", "metaCL17302A", "metaHisCL17302A"};
    private boolean runSuccess = false;

    @BeforeClass
    private void setUp() throws Exception {
        RegionUtils.createCSAndCL(csName, clNames);
        RegionUtils.deleteRegion(regionName1);
        RegionUtils.deleteRegion(regionName2);
    }

    @Test
    private void testStatic() throws Exception {
        //create region
        Region region = new Region();
        region.withDataLocation(csName + "." + clNames[0])
                .withMetaLocation(csName + "." + clNames[1])
                .withMetaHisLocation(csName + "." + clNames[2])
                .withName(regionName1);
        RegionUtils.putRegion(region);

        //delete region
        RegionUtils.deleteRegion(regionName1);

        //head region to make sure the region:regionName1 has been deleted
        Assert.assertFalse(RegionUtils.headRegion(regionName1), region.toString());

        //check cs.cl has not been deleted
        for (String clName : clNames) {
            Assert.assertTrue(RegionUtils.clInCS(csName, clName),
                    "csName = " + csName + ",clName = " + clName);
        }
    }

    @Test
    private void testDynamic() throws Exception {
        //create region
        Region region = new Region();
        region.withDataCSShardingType("year")
                .withDataCLShardingType("year")
                .withName(regionName2);
        RegionUtils.putRegion(region);

        //delete region
        RegionUtils.deleteRegion(regionName2);

        //head region to make sure the region:regionName1 has been deleted
        Assert.assertFalse(RegionUtils.headRegion(regionName2), region.toString());
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() throws Exception {
        if (runSuccess) {
            RegionUtils.dropCS(new String[]{csName});
            RegionUtils.deleteRegion(regionName1);
            RegionUtils.deleteRegion(regionName2);
        }
    }
}
