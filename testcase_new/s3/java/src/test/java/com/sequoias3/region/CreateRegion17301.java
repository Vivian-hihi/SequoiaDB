package com.sequoias3.region;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import java.io.IOException;


/**
 * @Description:  seqDB-17301 :: 创建区域配置不存在的domain
 * @author fanyu
 * @Date:2019年01月22日
 * @version:1.0
 */
public class CreateRegion17301 extends S3TestBase{
    private String[] regionNames = new String[]{"region17301a","region17301b","region17301c"};
    private String domainName = "doesNotExist17301";

    @BeforeClass
    private void setUp() throws IOException {
    }
    @DataProvider(name="range-provider")
    private Object[][] rangeData(){
       // List<String> domains = RegionUtils.getDomainNames();
        //if(domains.size() < 1){
           // throw new SkipException("domains.size() < 1,At least one domain is required");
       // }
        return new Object[][]{
//                {regionNames[0],domainName,domains.get(0)},
//                {regionNames[1],domains.get(0),domainName},
//                {regionNames[2],domainName,domainName}
                {regionNames[0],domainName,domainName},
                {regionNames[1],domainName,domainName},
                {regionNames[2],domainName,domainName}
        };
    }

    @Test(dataProvider = "range-provider")
    private void test(String regionName,String dataDomain, String metaDomain) throws Exception {
        //create region
        Region region = new Region();
        region.withDataCSShardingType("year").withDataCLShardingType("year")
                .withDataDomain(dataDomain)
                .withMetaDomain(metaDomain)
                .withName(regionName);
        try {
            RegionUtils.putRegion(region);
            Assert.fail("exp fail but act success,regionName = " +
                    "" + regionName + ",datadomain = " + dataDomain + ",metaDomain = " + metaDomain);
        } catch (AmazonS3Exception e) {
           if(e.getStatusCode() != 400 && !e.getErrorCode().contains("InvalidDomain")){
               throw e;
           }
        }
        //check region that was not created successfully
        Assert.assertFalse(RegionUtils.headRegion(regionName));
    }

    @AfterClass
    private void tearDown() throws Exception {
    }
}
