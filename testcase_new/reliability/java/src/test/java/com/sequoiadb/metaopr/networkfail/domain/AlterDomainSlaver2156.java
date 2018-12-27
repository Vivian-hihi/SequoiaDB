package com.sequoiadb.metaopr.networkfail.domain;

import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr ;
import com.sequoiadb.commlib.StandTestInterface;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.metaopr.commons.DBoperateTask;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;

import org.testng.SkipException ;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.List;

import static com.sequoiadb.metaopr.commons.MyUtil.*;
import static org.testng.Assert.assertEquals;
import static org.testng.Assert.assertTrue;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-4-28
 * @Version 1.00
 */
public class AlterDomainSlaver2156 implements StandTestInterface {
    final String domain = "domain2156";
    final String cs = "cs2156";
    final String cl = "cl2156";
    List<String> groupNames;

    /**
     * seqDB-2156 :: 版本: 1 :: 更新domain时catalog备节点断网_rlb.netSplit.metaOpr.domain.004
     * <p>
     * 1、创建domian，在域中添加两个数据组（如group1和group2），且设置AutoSplit参数为自动切分
     * 2、更新域属性（执行domain.alter（{Groups:[“group1”，“group3”}）），删除其中一个复制组，添加新复制组 （如group3）
     * 3、更新域属性过程中catalog备节点所在主机网络中断（构造网络中断故障，如ifdown网卡）
     * 3、查看domain更新结果和catalog备节点状态
     * 4、恢复网络故障（如ifup启动网卡）
     * 5、再次执行更新domain操作，并指定该domain创建CS/CL
     * 6、向CL插入数据，查看数据是否正确落到该域对应的组内
     * 7、查看catalog主备节点上该domain信息是否正确
     */
    @Test
    public void alterDomainSlaver() throws ReliabilityException {
        DBoperateTask task = DBoperateTask.getTaskAlterDomain(domain, 1000, groupNames);
        String hostName = getSlaveNodeOfCatalog().hostName();
        task.setHostname(CommLib.getSafeCoordUrl(hostName));
        FaultMakeTask faultMakeTask = BrokenNetwork.getFaultMakeTask(hostName, 0, 5);
        TaskMgr mgr = new TaskMgr(faultMakeTask, task);
        mgr.execute();

        checkBusiness();
        assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        alterDomain(domain, groupNames.get(1), groupNames.get(2));
        assertTrue(isCatalogGroupSync());

        //检查数据落组情况
        createCS(cs, domain);
        createCl(cs, cl);
        insertSimpleDataIntoCl(cs, cl, 10000);
        long count = 0;
        count += getClCountFromGroupMaster(groupNames.get(1), cs, cl);
        count += getClCountFromGroupMaster(groupNames.get(2), cs, cl);
        assertEquals(count, 10000);
    }

    @BeforeClass
    @Override
    public void setup() {
        printBeginTime(this);
        checkBusiness();
        try {
            groupNames = GroupMgr.getInstance().getAllDataGroupName();
        } catch ( ReliabilityException e ) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            throw new SkipException(e.getMessage()) ;
        }
        
        createDomainAutoSplit(domain, groupNames.get(0), groupNames.get(1));

    }

    @AfterClass
    @Override
    public void tearDown() {
        dropCS(cs);
        dropDomain(domain);
        printEndTime(this);
    }
}
