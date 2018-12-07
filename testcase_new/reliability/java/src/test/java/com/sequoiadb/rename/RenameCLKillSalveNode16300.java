package com.sequoiadb.rename;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BasicBSONObject;
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
 * @Description RenameCLKillSalveNode16300.java   seqDB-16300:执行rename cl过程中，数据备节点故障
 * @author luweikang
 * @date 2018年11月7日
 */
public class RenameCLKillSalveNode16300 extends SdbTestBase{
	
	private String csName = "cs16300A";
	private List<String> oldCLNameList = new ArrayList<>();
	private List<String> newCLNameList = new ArrayList<>();
	private String oldCLName = "oldCL_16300_A";
	private String newCLName = "newCL_16300_A";
	private GroupMgr groupMgr = null;
	private String groupName = null;
	private Sequoiadb sdb = null;
	private int csNum = 10;
	
	
	@BeforeClass
	public void setUp() throws ReliabilityException{
        System.out.println(
                "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                        + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        groupMgr = new GroupMgr();

        groupName = groupMgr.getAllDataGroupName().get(0);
        
        // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
        if (!groupMgr.checkBusiness(20)) {
            throw new SkipException("checkBusiness return false");
        }
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        for (int i = 0; i < csNum; i++) {
        	cs.createCollection(oldCLName + i, new BasicBSONObject("Group", groupName));
        	oldCLNameList.add(oldCLName + i);
        	newCLNameList.add(newCLName + i);
		}
	}
	
	@Test
    public void test() throws ReliabilityException {
        GroupWrapper dataGroup = groupMgr.getGroupByName( groupName );
        NodeWrapper dataSlave = dataGroup.getSlave();

        // 建立并行任务
        FaultMakeTask faultTask = KillNode.getFaultMakeTask(dataSlave.hostName(),
        		dataSlave.svcName(), 0);
        TaskMgr mgr = new TaskMgr(faultTask);
    	Rename renameTask = new Rename();
    	mgr.addTask(renameTask);
        mgr.execute();
        
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        Assert.assertTrue(groupMgr.checkBusiness(120));
        
        for (int i = 0; i < oldCLNameList.size(); i++) {
        	RenameUtils.retryRenameCL(csName, oldCLNameList.get(i), newCLNameList.get(i));
    		RenameUtils.checkRenameCLResult(sdb, csName, oldCLNameList.get(i), newCLNameList.get(i));
		}
        
        // 插入数据
        for (int i = 0; i < newCLNameList.size(); i++) {
        	DBCollection cl = sdb.getCollectionSpace(csName).getCollection(newCLNameList.get(i));
        	RenameUtils.insertData(cl, 1000);
        	long actNum = cl.getCount();
        	Assert.assertEquals(actNum, 1000, "check record num");
		}
        
        Assert.assertTrue(groupMgr.checkBusiness(120));
	}
	
	@AfterClass
    public void tearDown() {
		try {
			if(sdb.isCollectionSpaceExist(csName)){
				sdb.dropCollectionSpace(csName);
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
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
            	for (int i = 0; i < oldCLNameList.size(); i++) {
            		db.getCollectionSpace(csName).renameCollection(oldCLNameList.get(i), newCLNameList.get(i));
				}
            }catch(BaseException e){
            	Assert.assertEquals(e.getErrorCode(), -134, e.getMessage());
            }
        }
    }
}
