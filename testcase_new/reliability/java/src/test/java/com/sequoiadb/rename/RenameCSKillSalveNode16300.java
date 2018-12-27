package com.sequoiadb.rename;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
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

/**
 * @Description RenameCSKillSalveNode16300.java  seqDB-16300:执行rename过程中，数据备节点故障
 * @author luweikang
 * @date 2018年11月7日
 */
public class RenameCSKillSalveNode16300 extends SdbTestBase{
	
	private String oldCSName = "oldCS_16300B";
	private String newCSName = "newCS_16300B";
	private List<String> oldCSNameList = new ArrayList<>();
	private List<String> newCSNameList = new ArrayList<>();
	private String clName = "cl16300_B";
	private GroupMgr groupMgr = null;
	private Sequoiadb sdb = null;
	private int csNum = 10;
	
	
	@BeforeClass
	public void setUp() throws ReliabilityException{
        System.out.println(
                "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                        + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        groupMgr = GroupMgr.getInstance();

        // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
        if (!groupMgr.checkBusiness(20)) {
            throw new SkipException("checkBusiness return false");
        }

        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        for (int i = 0; i < csNum; i++) {
        	CollectionSpace cs = sdb.createCollectionSpace(oldCSName + i);
        	cs.createCollection(clName);
        	oldCSNameList.add(oldCSName + i);
        	newCSNameList.add(newCSName + i);
		}
	}
	
	@Test
    public void test() throws ReliabilityException {
        GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
        NodeWrapper cataMaster = cataGroup.getMaster();

        // 建立并行任务
        FaultMakeTask faultTask = KillNode.getFaultMakeTask(cataMaster.hostName(),
        		cataMaster.svcName(), 0);
        TaskMgr mgr = new TaskMgr(faultTask);
    	Rename renameTask = new Rename();
    	mgr.addTask(renameTask);
        mgr.execute();
        
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        Assert.assertTrue(groupMgr.checkBusiness(120));
        
        for (int i = 0; i < oldCSNameList.size(); i++) {
        	RenameUtils.retryRenameCS(oldCSNameList.get(i), newCSNameList.get(i));
    		RenameUtils.checkRenameCSResult(sdb, oldCSNameList.get(i), newCSNameList.get(i),1);
		}
        
        // 插入数据
        for (int i = 0; i < newCSNameList.size(); i++) {
        	DBCollection cl = sdb.getCollectionSpace(newCSNameList.get(i)).getCollection(clName);
        	RenameUtils.insertData(cl, 1000);
        	long actNum = cl.getCount();
        	Assert.assertEquals(actNum, 1000, "check record num");
		}
        
        Assert.assertTrue(groupMgr.checkBusiness(120));
	}
	
	@AfterClass
    public void tearDown() {
		try {
			for (int i = 0; i < newCSNameList.size(); i++) {
				String csName = newCSNameList.get(i);
				if(sdb.isCollectionSpaceExist(csName)){
					sdb.dropCollectionSpace(csName);
				}
			}
		} finally {
			if(sdb != null){
				sdb.close();
			}
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		}
	}
	
	class Rename extends OperateTask {
		
        @Override
        public void exec() throws Exception {
            try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
        		System.out.println(db.getHost());
        		System.out.println(db.getIP());
        		System.out.println(db.getLastUseTime());
        		System.out.println(db.getNodeName());
        		System.out.println(db.getPort());
            	for (int i = 0; i < oldCSNameList.size(); i++) {
            		db.renameCollectionSpace(oldCSNameList.get(i), newCSNameList.get(i));
				}
            }catch(BaseException e){
            	e.printStackTrace();
            	if(e.getErrorCode()!=-134){
            		throw e;
            	}
            }
        }
    }
}
